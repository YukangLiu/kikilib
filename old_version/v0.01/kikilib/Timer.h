//@Author Liu Yukang 
#pragma once
#include "Time.h"
#include "Socket.h"
#include "utils.h"

#include <map>
#include <mutex>
#include <functional>

namespace kikilib
{
	//定时器
	//职责：
	//1、管理 某一时刻——该时刻需要执行的函数 的映射表
	//2、提供定时事件的操作接口
	//3、需要保证线程安全，因为主线程执行的HandleConnectionEvent函数和EventManager可能同时使用同一个定时器
	class Timer
	{
	public:
		Timer(Socket timeFd) : _timeSock(timeFd) {}
		~Timer() {}

		DISALLOW_COPY_MOVE_AND_ASSIGN(Timer);

		//运行所有已经超时的需要执行的函数
		void RunExpired();

		//在time时刻需要执行函数cb
		void RunAt(Time time, std::function<void()> cb);

	private:
		Socket _timeSock;

		std::mutex _timerMutex;

		//定时器回调函数集合
		std::multimap<Time, std::function<void()>> _timerCbMap;
	};

}
