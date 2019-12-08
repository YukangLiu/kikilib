//@Author Liu Yukang 
#pragma once

#include "Socket.h"
#include "utils.h"

#include <sys/types.h>
#include <vector>
#include <string>
#include <stdint.h>

namespace kikilib
{
	//socket读取器
	//职责：
	//1、提供读取socket接收缓冲区内容的API
	//2、并提供一定的用户缓冲区缓存用户还没读的内容
	class SocketReader
	{
	public:
		SocketReader(Socket& sock);
		SocketReader(Socket&& sock);

		~SocketReader() {};

		DISALLOW_COPY_MOVE_AND_ASSIGN(SocketReader);

		//缓冲区是否为空
		bool IsEmptyAfterRead();

		//读取一个int，若缓存中没有，则返回false
		bool ReadInt32(int& res);

		//读取一个int64，若缓存中没有，则返回false，未来实现
		//bool ReadInt64(int64_t& res);

		//读取长度为len的数据，若没有长度为len的数据，则返回空串
		std::string Read(size_t len);

		//读一行，该行以\r\n结尾,若没有，返回空串
		std::string ReadLineEndOfRN();

		//读一行，该行以\r结尾,若没有，返回空串
		std::string ReadLineEndOfR();

		//读一行，该行以\n结尾,若没有，返回空串
		std::string ReadLineEndOfN();

		//读取所有能读取的数据，没有则返回空串
		std::string ReadAll();
		

	private:

		//尝试读取能填满缓冲区的数据,若缓冲区已经满了，会先扩充1.5倍大小
		ssize_t ReadFillBuf();

		Socket _sock;

		//自身缓冲区的左边界
		size_t _leftBorder;

		//自身缓冲区的右边界
		size_t _rightBorder;

		//自身缓冲区
		std::vector<char> _buffer;

	};

}