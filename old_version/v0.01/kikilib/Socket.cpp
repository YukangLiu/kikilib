//@Author Liu Yukang 
#include "Socket.h"
#include "LogManager.h"

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdio.h>  // snprintf
#include <fcntl.h>
#include <errno.h>

using namespace kikilib;

Socket::~Socket()
{
	--(*_pRef);
	if (!(*_pRef) && IsUseful())
	{
		if (::close(_sockfd) < 0)
		{
			RecordLog(ERROR_DATA_INFORMATION, std::string("sockets::close errno : ") + std::to_string(errno));
		}
		else
		{
			//RecordLog(std::string(std::to_string(_sockfd) + " close"));
		}
		delete _pRef;
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


int Socket::Bind(int port)
{
	_port = port;
	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(struct sockaddr_in));
	serv.sin_family = AF_INET;
	serv.sin_port = htons(port);
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	int ret = ::bind(_sockfd, (struct sockaddr*) & serv, sizeof(serv));
	if (ret < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("Socket::Bind errno : ") + std::to_string(errno));
	}
	return ret;
}

int Socket::Listen()
{
	int ret = ::listen(_sockfd, Parameter::backLog);
	if (ret < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("Socket::Listen failed. errno : ") + std::to_string(errno));
	}
	return ret;
}

Socket Socket::Accept()
{
	int connfd = -1;
	struct sockaddr_in client;
	socklen_t len = sizeof(client);
	connfd = ::accept(_sockfd, (struct sockaddr*) & client, &len);
	if (connfd < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("Socket::accept failed. errno : ") + std::to_string(errno));
		return Socket(connfd);
	}

	SetTcpNoDelay(Parameter::isNoDelay);

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

int Socket::ShutdownWrite()
{
	int ret = ::shutdown(_sockfd, SHUT_WR);
	if (ret < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("ShutdownWrite failed. errno : ") + std::to_string(errno));
	}
	return ret;
}

int Socket::SetTcpNoDelay(bool on)
{
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(_sockfd, IPPROTO_TCP, TCP_NODELAY,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0 && on)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("SetTcpNoDelay failed. errno : ") + std::to_string(errno));
	}
	return ret;
}

int Socket::SetReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0 && on)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("SetReuseAddr failed. errno : ") + std::to_string(errno));
	}
	return ret;
}

int Socket::SetReusePort(bool on)
{
	int ret = -1;
#ifdef SO_REUSEPORT
	int optval = on ? 1 : 0;
	ret = ::setsockopt(_sockfd, SOL_SOCKET, SO_REUSEPORT,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0 && on)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("SetReusePort failed. errno : ") + std::to_string(errno));
	}
#else
	if (on)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("SO_REUSEPORT is not supported. errno : ") + std::to_string(errno));
	}
#endif
	return ret;
}

int Socket::SetKeepAlive(bool on)
{
	int optval = on ? 1 : 0;
	int ret = ::setsockopt(_sockfd, SOL_SOCKET, SO_KEEPALIVE,
		&optval, static_cast<socklen_t>(sizeof optval));
	if (ret < 0 && on)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("SetKeepAlive failed. errno : ") + std::to_string(errno));
	}
	return ret;
}

//设置socket为非阻塞的
int Socket::SetNonBolckSocket()
{
	auto flags = fcntl(_sockfd, F_GETFL, 0);
	int ret = fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK);   //设置成非阻塞模式
	if (ret < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("SetNonBolckSocket failed. errno : ") + std::to_string(errno));
	}
	return ret;
}

//设置socket为阻塞的
int Socket::SetBlockSocket()
{
	auto flags = fcntl(_sockfd, F_GETFL, 0);
	int ret = fcntl(_sockfd, F_SETFL, flags & ~O_NONBLOCK);    //设置成阻塞模式；
	if (ret < 0 )
	{
		RecordLog(ERROR_DATA_INFORMATION,std::string("SetBlockSocket failed. errno : ") + std::to_string(errno));
	}
	return ret;
}

//void Socket::SetNoSigPipe()
//{
   // int set = 1;
   // ::setsockopt(_sockfd, SOL_SOCKET, MSG_NOSIGNAL, (void *)&set, static_cast<socklen_t>(sizeof(int)));
//}