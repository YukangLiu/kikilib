//@Author Liu Yukang 
#pragma once
#include "Time.h"
#include "Socket.h"
#include "utils.h"

#include <map>
#include <vector>
#include <mutex>
#include <functional>

namespace kikilib
{
	//定时器
	//职责：
	//1、管理 某一时刻——该时刻需要执行的函数 的映射表
	//2、提供定时事件的操作接口
	//3、该类不保证线程安全，线程安全由EventManager负责
	class Timer
	{
	public:
		Timer(Socket& timeSock) : _timeSock(timeSock) {}
		Timer(Socket&& timeSock) : _timeSock(timeSock) {}
		~Timer() {}

		DISALLOW_COPY_MOVE_AND_ASSIGN(Timer);

		//获取所有已经超时的需要执行的函数
		void getExpiredTask(std::vector<std::function<void()>> &tasks);

		//在time时刻需要执行函数cb
		void runAt(Time time, std::function<void()>& cb);
		void runAt(Time time, std::function<void()>&& cb);

	private:
		//给timefd重新设置时间，time是绝对时间
		void resetTimeOfTimefd(Time time);

		Socket _timeSock;

		//定时器回调函数集合
		std::map<Time, std::function<void()>> _timerCbMap;
	};

}
