#include "Socket.h"
#include "LogManager.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>  // snprintf
#include <fcntl.h>

using namespace kikilib;

Socket::~Socket()
{
	--(*_pRef);
	if (!(*_pRef))
	{
		if (::close(_sockfd) < 0)
		{
			RecordLog(ERROR_DATA_INFORMATION, "sockets::close");
		}
        //printf("one usr closed \n");
        RecordLog(std::string(std::to_string(_sockfd) + " close") );
	}
}

bool Socket::GetSocketOpt(struct tcp_info* tcpi) const
{
	socklen_t len = sizeof(*tcpi);
	memset(tcpi, 0, sizeof(*tcpi));
	return ::getsockopt(_sockfd, SOL_TCP, TCP_INFO, tcpi, &len) == 0;
}

bool Socket::GetSocketOptString(char* buf, int len) const
{
	struct tcp_info tcpi;
	bool ok = GetSocketOpt(&tcpi);
	if (ok)
	{
		snprintf(buf, len, "unrecovered=%u "
			"rto=%u ato=%u snd_mss=%u rcv_mss=%u "
			"lost=%u retrans=%u rtt=%u rttvar=%u "
			"sshthresh=%u cwnd=%u total_retrans=%u",
			tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
			tcpi.tcpi_rto,          // Retransmit timeout in usec
			tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
			tcpi.tcpi_snd_mss,
			tcpi.tcpi_rcv_mss,
			tcpi.tcpi_lost,         // Lost packets
			tcpi.tcpi_retrans,      // Retransmitted packets out
			tcpi.tcpi_rtt,          // Smoothed round trip time in usec
			tcpi.tcpi_rttvar,       // Medium deviation
			tcpi.tcpi_snd_ssthresh,
			tcpi.tcpi_snd_cwnd,
			tcpi.tcpi_total_retrans);  // Total retransmits for entire connection
	}
	return ok;
}

std::string Socket::GetSocketOptString() const
{
	char buf[1024];
	buf[0] = '\0';
	GetSocketOptString(buf, sizeof buf);
	return std::string(buf);
}


void Socket::Bind(std::string& ip, int port)
{
	_ip = ip;
	_port = port;
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = inet_addr(ip.c_str());

	while (::bind(_sockfd, (struct sockaddr*) & serv, sizeof(serv)) < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, "Socket::Bind");
	}
}

void Socket::Listen()
{
	if (::listen(_sockfd, Parameter::backLog) < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, "Socket::Listen");
	}
}

Socket Socket::Accept()
{
	int connfd = -1;
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	connfd = ::accept(_sockfd, (struct sockaddr*) & client, &len);

	//accept成功保存用户ip
	struct sockaddr_in* sock = (struct sockaddr_in*) & client;
	int port = ntohs(sock->sin_port);          //linux上打印方式
	struct in_addr in = sock->sin_addr;
	char ip[INET_ADDRSTRLEN];   //INET_ADDRSTRLEN这个宏系统默认定义 16
	//成功的话此时IP地址保存在str字符串中。
	inet_ntop(AF_INET, &in, ip, sizeof(ip));

	return Socket(connfd, std::string(ip), port);
}

//从socket中读数据
ssize_t Socket::Read(void* buf, size_t count)
{
	return ::read(_sockfd, buf, count);
}

//往socket中写数据
ssize_t Socket::Send(const void* buf, size_t count)
{
	//忽略SIGPIPE信号
	return ::send(_sockfd, buf, count, MSG_NOSIGNAL);
}

void Socket::ShutdownWrite()
{
	if (::shutdown(_sockfd, SHUT_WR) < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, "shutdownWrite");
	}
}

void Socket::SetTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY,
		&optval, static_cast<socklen_t>(sizeof optval));
	// FIXME CHECK
}

void Socket::SetReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0 && on)
	{
		RecordLog(ERROR_DATA_INFORMATION, "SO_REUSEPORT failed.");
	}
	// FIXME CHECK
}

void Socket::SetReusePort(bool on)
{
#ifdef SO_REUSEPORT
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0 && on)
	{
		RecordLog(ERROR_DATA_INFORMATION, "SO_REUSEPORT failed.");
	}
#else
	if (on)
	{
		RecordLog(ERROR_DATA_INFORMATION, "SO_REUSEPORT is not supported.");
	}
#endif
}

void Socket::SetKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
	::setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE,
		&optval, static_cast<socklen_t>(sizeof optval));
	// FIXME CHECK
}

//设置socket为非阻塞的
void Socket::SetNonBolckSocket()
{
	auto flags = fcntl(_sockfd, F_GETFL, 0);
	fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK);   //设置成非阻塞模式
}

//设置socket为阻塞的
void Socket::SetBlockSocket()
{
	auto flags = fcntl(_sockfd, F_GETFL, 0);
	fcntl(_sockfd, F_SETFL, flags & ~O_NONBLOCK);    //设置成阻塞模式；
}

//void Socket::SetNoSigPipe()
//{
   // int set = 1;
   // ::setsockopt(_sockfd, SOL_SOCKET, MSG_NOSIGNAL, (void *)&set, static_cast<socklen_t>(sizeof(int)));
//}