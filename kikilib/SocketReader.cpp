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
	auto ret = readFillBuf();
	return ret == 0;
}

//��ȡһ��int����������û�У��򷵻�false
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

//��ȡһ��int64����������û�У��򷵻�false,�Ժ�ʵ��
//bool SocketReader::ReadInt64(int64_t& res)
//{
//	ReadFillBuf();
//
//}

//��ȡ����Ϊlen�����ݣ���û�г���Ϊlen�����ݣ��򷵻ؿմ�
std::string SocketReader::read(size_t len)
{
	//�������ڷ��ش�����string��Ϊ������RVO����
	//buffer���Ѿ����㹻��������
	if (_rightBorder - _leftBorder >= len)
	{
		_leftBorder += len;
		return std::string(_buffer.begin() + _leftBorder - len, _buffer.begin() + _leftBorder);
	}
	if (_buffer.size() - _leftBorder < len)
	{//buffer�ռ䲻���Խ��ճ���len�����ݣ���չ���ոտ���
		_buffer.resize(len + _leftBorder);
	}
	readFillBuf();
	if (_rightBorder - _leftBorder >= len)
	{//��ȡ֮�����㹻��������
		_leftBorder += len;
		return std::string(_buffer.begin() + _leftBorder - len, _buffer.begin() + _leftBorder);
	}
	//��ȡ֮����û���㹻������
	return std::string("");
}

//��ȡ����Ϊlen�����ݣ���û�г���Ϊlen�����ݣ��򷵻�false
bool SocketReader::read(char* buf, size_t len)
{
	//buffer���Ѿ����㹻��������
	if (_rightBorder - _leftBorder >= len)
	{
		memcpy(buf,&_buffer[_leftBorder],len);
		_leftBorder += len;
		return true;
	}
	if (_buffer.size() - _leftBorder < len)
	{//buffer�ռ䲻���Խ��ճ���len�����ݣ���չ���ոտ���
		_buffer.resize(len + _leftBorder);
	}
	readFillBuf();
	if (_rightBorder - _leftBorder >= len)
	{//��ȡ֮�����㹻��������
		memcpy(buf, &_buffer[_leftBorder], len);
		_leftBorder += len;
		return true;
	}
	//��ȡ֮����û���㹻������
	return false;
}

//���Զ�ȡ������������������,���������Ѿ����ˣ���������Parameter::bufExpandRatio����С
ssize_t SocketReader::readFillBuf()
{
	if (_leftBorder >= (_buffer.size() / Parameter::bufMoveCriterion))
	{//����ߵĿ���λ��̫��
		memmove(&(_buffer.front()), &(_buffer[_leftBorder]), _rightBorder - _leftBorder);
		_rightBorder -= _leftBorder;
		_leftBorder = 0;
	}

	if (_rightBorder == _buffer.size())
	{//������������
		_buffer.resize(static_cast<size_t>(static_cast<double>(_buffer.size())* Parameter::bufExpandRatio));
	}

	ssize_t ret = _sock.read(&(_buffer[_rightBorder]), _buffer.size() - _rightBorder);

	if (ret > 0)
	{
		_rightBorder += ret;
	}
	return ret;
}

//��ȡ�����ܶ�ȡ�����ݣ�û���򷵻ؿմ�
std::string SocketReader::readAll()
{
	//һֱ����û��������
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

//��һ�У�������\r\n��β
std::string SocketReader::readLineEndOfRN()
{
	//ReadFillBuf();
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder, endPos = tmpLeft;
	for (; endPos < tmpRight; ++endPos)
	{
		if (_buffer[endPos] == '\r' && endPos + 1 < tmpRight && _buffer[endPos + 1] == '\n')
		{
			//�ҵ�\r\n
			endPos += 2;
			_leftBorder = endPos;
			return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + endPos);
		}
	}
	//û��\r\n
	//_rightBorder == _buffer.size()��ζ�ſ��ܻ���socket����û����
	if (_rightBorder == _buffer.size() && readFillBuf() > 0)
	{	
		//socket��������������û���꣬�ٴγ���Ѱ��\r\n
		return readLineEndOfRN();
	}
	//socket�����˶�û����\r\n
	return std::string("");
}

//��һ�У�������\r��β
std::string SocketReader::readLineEndOfR()
{
	//ReadFillBuf();
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder, endPos = tmpLeft;
	for (; endPos < tmpRight; ++endPos)
	{
		if (_buffer[endPos] == '\r')
		{
			//�ҵ�\r
			++endPos;
			_leftBorder = endPos;
			return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + endPos);
		}
	}
	//û��\r
	//_rightBorder == _buffer.size()��ζ�ſ��ܻ���socket����û����
	if (_rightBorder == _buffer.size() && readFillBuf() > 0)
	{
		//socket��������������û���꣬�ٴγ���Ѱ��\r
		return readLineEndOfR();
	}
	//socket�����˶�û����\r
	return std::string("");
}

//��һ�У�������\n��β
std::string SocketReader::readLineEndOfN()
{
	//ReadFillBuf();
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder, endPos = tmpLeft;
	for (; endPos < tmpRight; ++endPos)
	{
		if (_buffer[endPos] == '\n')
		{
			//�ҵ�\n
			++endPos;
			_leftBorder = endPos;
			return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + endPos);
		}
	}
	//û��\n
	//_rightBorder == _buffer.size()��ζ�ſ��ܻ���socket����û����
	if (_rightBorder == _buffer.size() && readFillBuf() > 0)
	{
		//socket��������������û���꣬�ٴγ���Ѱ��\r
		return readLineEndOfR();
	}
	//socket�����˶�û����\n
	return std::string("");
}
