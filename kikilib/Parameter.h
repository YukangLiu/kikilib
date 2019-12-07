//@Author Liu Yukang 
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

		//日志在占内存的最大大小（Byte），大于该数会舍弃掉一部分日志内容，防止内存爆满
		//const int64_t maxLogQueueByte = 1073741824; //1024 * 1024 * 1024 * 1, 1GB

		//日志在占磁盘的最大大小（Byte），大于该数会舍弃掉一个日志文件，重新开始写，防止磁盘爆满
		const int64_t maxLogDiskByte = 1073741824; //1024 * 1024 * 1024, 1GB

		//日志中的ringbuffer的长度，需要是2的n次幂,一个std::string大小是40字节，加上字符串假设60字节，
		//所以该数字*100可以认为是内存允许分给日志系统的大小，如4194304表示内存最多给日志419MB空间
		const int64_t kLogBufferLen = 4194304;

		static_assert(((kLogBufferLen > 0) && ((kLogBufferLen& (~kLogBufferLen + 1)) == kLogBufferLen)),
			"RingBuffer's size must be a positive power of 2");

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