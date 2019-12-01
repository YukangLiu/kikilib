#pragma once

#include <string>
#include <string.h>

namespace kikilib
{
	//参数列表
	namespace Parameter
	{
		//日志名字
		const static std::string logName("Log.txt");

		//线程池中线程的数量
		constexpr static unsigned threadPoolCnt = 4;

		//监听队列的长度
		constexpr static unsigned backLog = 1024;

		//Socket默认是否开启TCP_NoDelay
		constexpr static bool isNoDelay = true;

		//获取活跃的epoll_event的数组的初始长度
		static constexpr int epollEventListFirstSize = 16;

		//epoll_wait的阻塞时常
		static constexpr int epollTimeOutMs = 10000;

		//SocketReader和SocketWritter中缓冲区的初始大小
		static constexpr size_t bufferInitLen = 1024;

		//SocketReader和SocketWritter中的缓冲区，
		//若前面空闲的位置大于了buffer总大小的1 / bufMoveCriterion，
		//则自动向前补齐
		static constexpr size_t bufMoveCriterion = 3;

		//SocketReader和SocketWritter中的缓冲区，
		//若已经满了，但是需要读数据，会先扩展buffer的size为当前的bufExpandRatio倍
		static constexpr double bufExpandRatio = 1.5;
	};
}