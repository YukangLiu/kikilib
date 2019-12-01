#pragma once

#include "utils.h"
#include "Parameter.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <sys/socket.h>

struct tcp_info;

namespace kikilib {

	//Socket类，创建的Socket对象默认都是非阻塞,端口复用的
	//职责：
	//1、提供fd操作的相关API
	//2、管理fd的生命周期
	//其中有引用计数，若某一fd没人用了就会close
	class Socket
	{
	public:
		explicit Socket(int sockfd, std::string ip = "", int port = -1)
			: _sockfd(sockfd), _ip(std::move(ip)), _port(port)
		{
			_pRef = new int(1);
			if (sockfd > 0)
			{
				SetTcpNoDelay(Parameter::isNoDelay);
				SetNonBolckSocket();
				SetReuseAddr(true);
				SetReusePort(true);
			}
		}

		Socket()
			: _sockfd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)), _ip(""), _port(-1)
		{
			_pRef = new int(1);
			if (_sockfd > 0)
			{
				SetTcpNoDelay(Parameter::isNoDelay);
				SetReuseAddr(true);
				SetReusePort(true);
			}
		}

		Socket(const Socket& otherSock) : _sockfd(otherSock._sockfd)
		{
			*(otherSock._pRef) += 1;
			_pRef = otherSock._pRef;
			_ip = otherSock._ip;
			_port = otherSock._port;
		}

		Socket(Socket&& otherSock) : _sockfd(otherSock._sockfd)
		{
			*(otherSock._pRef) += 1;
			_pRef = otherSock._pRef;
			_ip = otherSock._ip;
			_port = otherSock._port;
		}

		Socket& operator=(const Socket& otherSock) = delete;

		~Socket();

		//返回当前Socket的fd
		int fd() const { return _sockfd; }

		//绑定ip和port到当前Socket
		void Bind(std::string& ip, int port);

		//开始监听当前Socket
		void Listen();

		//阻塞接收一个连接，返回一个新连接的Socket
		Socket Accept();

		//从socket中读数据
		ssize_t Read(void* buf, size_t count);

		//往socket中写数据
		ssize_t Send(const void* buf, size_t count);

		//获取当前套接字的目标ip
		std::string GetIp() { return _ip; }

		//获取当前套接字的目标port
		int GetPort() { return _port; }

		//获取套接字的选项,成功则返回true，反之，返回false
		bool GetSocketOpt(struct tcp_info*) const;

		//获取套接字的选项的字符串,成功则返回true，反之，返回false
		bool GetSocketOptString(char* buf, int len) const;

		//获取套接字的选项的字符串
		std::string GetSocketOptString() const;

		//关闭套接字的写操作
		void ShutdownWrite();

		//设置是否开启Nagle算法减少需要传输的数据包，若开启延时可能会增加
		void SetTcpNoDelay(bool on);

		//设置是否地址重用
		void SetReuseAddr(bool on);

		//设置是否端口重用
		void SetReusePort(bool on);

		//设置是否使用心跳检测
		void SetKeepAlive(bool on);

		//设置socket为非阻塞的
		void SetNonBolckSocket();

		//设置socket为阻塞的
		void SetBlockSocket();

		//void SetNoSigPipe();

	private:
		//fd
		const int _sockfd;

		//引用计数
		int* _pRef;

		//端口号
		int _port;

		//ip
		std::string _ip;
	};

}  // namespace kikilib