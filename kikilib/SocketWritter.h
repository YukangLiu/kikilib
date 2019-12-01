#pragma once

#include "Socket.h"
#include "Parameter.h"
#include "utils.h"

#include <sys/types.h>
#include <vector>
#include <string>
#include <stdint.h>

namespace kikilib
{
	class EventService;

	//socket书写器
	//职责：
	//1、提供写socket发送缓冲区内容的API
	//2、并提供一定的用户缓冲区缓存用户一次因为各种原因没写完的内容
	class SocketWtitter
	{
	public:
		SocketWtitter(Socket sock, EventService* pEvServe);

		~SocketWtitter() {};

		DISALLOW_COPY_MOVE_AND_ASSIGN(SocketWtitter);

		//发送一个int
		void SendInt32(int res);

		//发送一个字符串
		void Send(std::string& str);

		//发送一个字符串
		void Send(std::string&& str);

		//将缓冲区内容写进socket中
		void WriteBufToSock();

	private:

		Socket _sock;

		//自身缓冲区的左边界
		int _leftBorder;

		//自身缓冲区的右边界
		int _rightBorder;

		//自身所属的事件服务
		EventService* _pEvServe;

		//自身缓冲区
		std::vector<char> _buffer;

	};

}