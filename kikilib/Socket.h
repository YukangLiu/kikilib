//@Author Liu Yukang 
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

	//Socket类，创建的Socket对象默认都是非阻塞的
	//职责：
	//1、提供fd操作的相关API
	//2、管理fd的生命周期
	//其中有引用计数，若某一fd没人用了就会close
	class Socket
	{
	public:
		explicit Socket(int sockfd, std::string ip = "", int port = -1)
			: _sockfd(sockfd), _pRef(new int(1)), _port(port), _ip(std::move(ip))
		{
			if (sockfd > 0)
			{
				SetNonBolckSocket();
			}
		}

		Socket()
			: _sockfd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)),
			_pRef(new int(1)), _port(-1), _ip("")
		{ }

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
			_ip = std::move(otherSock._ip);
			_port = otherSock._port;
		}

		Socket& operator=(const Socket& otherSock) = delete;

		~Socket();

		//返回当前Socket的fd
		int fd() const { return _sockfd; }

		//返回当前Socket是否可用
		bool IsUseful() { return _sockfd >= 0; }

		//绑定ip和port到当前Socket
		int Bind(int port);

		//开始监听当前Socket
		int Listen();

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
		int ShutdownWrite();

		//设置是否开启Nagle算法减少需要传输的数据包，若开启延时可能会增加
		int SetTcpNoDelay(bool on);

		//设置是否地址重用
		int SetReuseAddr(bool on);

		//设置是否端口重用
		int SetReusePort(bool on);

		//设置是否使用心跳检测
		int SetKeepAlive(bool on);

		//设置socket为非阻塞的
		int SetNonBolckSocket();

		//设置socket为阻塞的
		int SetBlockSocket();

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