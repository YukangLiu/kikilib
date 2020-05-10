//@Author Liu Yukang 
#include "SocketReader.h"
#include "Parameter.h"

#include <string.h>
#include <stdlib.h>

using namespace kikilib;

SocketReader::SocketReader(Socket& sock)
	: _sock(sock), _leftBorder(0), _rightBorder(0)
{
	_buffer.resize(Parameter::bufferInitLen);
}

SocketReader::SocketReader(Socket&& sock)
	: _sock(sock), _leftBorder(0), _rightBorder(0)
{
	_buffer.resize(Parameter::bufferInitLen);
}

bool SocketReader::isEmptyAfterRead()
{
	if (_rightBorder == _leftBorder)
	{
		auto ret = readFillBuf();
		if (ret <= 0)
		{
			return true;
		}
	}
	return false;
}

//读取一个int，若缓存中没有，则返回false
bool SocketReader::readInt32(int& res)
{
	size_t numSize = 0;
	char* numEnd = &_buffer[_leftBorder];
	res = std::strtol(&_buffer[_leftBorder], &numEnd,10);
	numSize = numEnd - &_buffer[_leftBorder];
	if (!numSize || numSize > (_rightBorder - _leftBorder))
	{
		return false;
	}
	_leftBorder += numSize;
	return true;
}

//读取一个int64，若缓存中没有，则返回false,以后实现
//bool SocketReader::ReadInt64(int64_t& res)
//{
//	ReadFillBuf();
//
//}

//读取长度为len的数据，若没有长度为len的数据，则返回空串
std::string SocketReader::read(size_t len)
{
	//函数中在返回处构造string是为了满足RVO条件
	//buffer中已经有足够的数据了
	if (_rightBorder - _leftBorder >= len)
	{
		_leftBorder += len;
		return std::string(_buffer.begin() + _leftBorder - len, _buffer.begin() + _leftBorder);
	}
	if (_buffer.size() - _leftBorder < len)
	{//buffer空间不足以接收长度len的数据，扩展到刚刚可以
		_buffer.resize(len + _leftBorder);
	}
	readFillBuf();
	if (_rightBorder - _leftBorder >= len)
	{//读取之后有足够的数据了
		_leftBorder += len;
		return std::string(_buffer.begin() + _leftBorder - len, _buffer.begin() + _leftBorder);
	}
	//读取之后还是没有足够的数据
	return std::string("");
}

//读取长度为len的数据，若没有长度为len的数据，则返回false
bool SocketReader::read(char* buf, size_t len)
{
	//buffer中已经有足够的数据了
	if (_rightBorder - _leftBorder >= len)
	{
		memcpy(buf,&_buffer[_leftBorder],len);
		_leftBorder += len;
		return true;
	}
	if (_buffer.size() - _leftBorder < len)
	{//buffer空间不足以接收长度len的数据，扩展到刚刚可以
		_buffer.resize(len + _leftBorder);
	}
	readFillBuf();
	if (_rightBorder - _leftBorder >= len)
	{//读取之后有足够的数据了
		memcpy(buf, &_buffer[_leftBorder], len);
		_leftBorder += len;
		return true;
	}
	//读取之后还是没有足够的数据
	return false;
}

//尝试读取能填满缓冲区的数据,若缓冲区已经满了，会先扩充Parameter::bufExpandRatio倍大小
ssize_t SocketReader::readFillBuf()
{
	if (_leftBorder >= (_buffer.size() / Parameter::bufMoveCriterion))
	{//若左边的空闲位置太多
		memmove(&(_buffer.front()), &(_buffer[_leftBorder]), _rightBorder - _leftBorder);
		_rightBorder -= _leftBorder;
		_leftBorder = 0;
	}

	if (_rightBorder == _buffer.size())
	{//若缓冲区已满
		_buffer.resize(static_cast<size_t>(static_cast<double>(_buffer.size())* Parameter::bufExpandRatio));
	}

	ssize_t ret = _sock.read(&(_buffer[_rightBorder]), _buffer.size() - _rightBorder);

	if (ret > 0)
	{
		_rightBorder += ret;
	}
	return ret;
}

//读取所有能读取的数据，没有则返回空串
std::string SocketReader::readAll()
{
	//一直读到没东西读了
	while (readFillBuf() > 0)
	{
		if (_rightBorder != _buffer.size())
		{
			break;
		}
	}
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder;
	_rightBorder = 0;
	_leftBorder = 0;
	return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + tmpRight);
}

//读一行，该行以\r\n结尾
std::string SocketReader::readLineEndOfRN()
{
	//ReadFillBuf();
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder, endPos = tmpLeft;
	for (; endPos < tmpRight; ++endPos)
	{
		if (_buffer[endPos] == '\r' && endPos + 1 < tmpRight && _buffer[endPos + 1] == '\n')
		{
			//找到\r\n
			endPos += 2;
			_leftBorder = endPos;
			return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + endPos);
		}
	}
	//没有\r\n
	//_rightBorder == _buffer.size()意味着可能还有socket数据没读完
	if (_rightBorder == _buffer.size() && readFillBuf() > 0)
	{	
		//socket缓冲区还有内容没读完，再次尝试寻找\r\n
		return readLineEndOfRN();
	}
	//socket读完了都没发现\r\n
	return std::string("");
}

//读一行，该行以\r结尾
std::string SocketReader::readLineEndOfR()
{
	//ReadFillBuf();
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder, endPos = tmpLeft;
	for (; endPos < tmpRight; ++endPos)
	{
		if (_buffer[endPos] == '\r')
		{
			//找到\r
			++endPos;
			_leftBorder = endPos;
			return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + endPos);
		}
	}
	//没有\r
	//_rightBorder == _buffer.size()意味着可能还有socket数据没读完
	if (_rightBorder == _buffer.size() && readFillBuf() > 0)
	{
		//socket缓冲区还有内容没读完，再次尝试寻找\r
		return readLineEndOfR();
	}
	//socket读完了都没发现\r
	return std::string("");
}

//读一行，该行以\n结尾
std::string SocketReader::readLineEndOfN()
{
	//ReadFillBuf();
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder, endPos = tmpLeft;
	for (; endPos < tmpRight; ++endPos)
	{
		if (_buffer[endPos] == '\n')
		{
			//找到\n
			++endPos;
			_leftBorder = endPos;
			return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + endPos);
		}
	}
	//没有\n
	//_rightBorder == _buffer.size()意味着可能还有socket数据没读完
	if (_rightBorder == _buffer.size() && readFillBuf() > 0)
	{
		//socket缓冲区还有内容没读完，再次尝试寻找\r
		return readLineEndOfR();
	}
	//socket读完了都没发现\n
	return std::string("");
}
