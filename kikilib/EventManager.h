//@Author Liu Yukang 
#pragma once

#include "EventEpoller.h"
#include "Time.h"
#include "utils.h"

#include <set>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <future>

namespace kikilib
{
	class EventService;
	class Timer;
	class ThreadPool;

	//事件优先级
	enum EventPriority
	{
		NORMAL_EVENT = 0,  //一般事件
		IMMEDIATE_EVENT,   //紧急事件
		EVENT_PRIORITY_TYPE_COUNT   //事件优先级种类个数
	};

	//事件管理器
	//职责：
	//1、提供向其中插入事件，移除，修改事件的接口
	//2、提供定时器的使用接口
	//3、循环扫描其管理的事件中被激活的对象，然后根据事件类型调用其相关函数
	//需要管理以下类的生命：
	//1、循环器线程looper
	//2、事件监视器epoller
	//3、定时器timer
	//4、所有的事件EventService
	class EventManager
	{
	public:
		EventManager(int idx, ThreadPool* threadPool);
		~EventManager();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventManager);

		//创建一个线程，然后线程中循环扫描事件
		bool loop();

		//向事件管理器中插入一个事件,这是线程安全的
		void insertEv(EventService* ev);

		//向事件管理器中移除一个事件,这是线程安全的
		void removeEv(EventService* ev);

		//向事件管理器中修改一个事件服务所关注的事件类型,这是线程安全的
		void modifyEv(EventService* ev);

		//需要注意，如果timerCb里面会执行RunExpired()函数的话会发生死锁
		//在time时刻执行timerCb函数
		//一个time就八个字节，搞引用相当于一个指针还是分配了八个字节（x64），所以time不搞&和&&
		void runAt(Time time, std::function<void()>&& timerCb);
		void runAt(Time time, std::function<void()>& timerCb);

		//time时间后执行timerCb函数
		void runAfter(Time time, std::function<void()>&& timerCb);
		void runAfter(Time time, std::function<void()>& timerCb);

		//每过time时间执行一次timerCb函数
		void runEvery(Time time, std::function<void()> timerCb);

		//每过time时间执行一次timerCb函数,直到isContinue函数返回false
		void runEveryUntil(Time time, std::function<void()> timerCb, std::function<bool()> isContinue);

		//运行所有已经超时的需要执行的函数
		void runExpired();

		//将任务函数放在线程池中以达到异步执行的效果
        void runInThreadPool(std::function<void()>&& func);

		//设置EventManager区域唯一的上下文内容
		//考虑如下场景：
		//每个EventManager中所有的事件需要共享一个LRU缓冲区的时候而这个LRU队列
		//又是属于每个EventManager而非全局的，那么就需要设置这个上下文指针了。
		//EventManager不负责管理该对象的生命，默认为nullptr
		void setEvMgrCtx(void* ctx);

		//获得EventManager区域唯一的上下文内容
		void* getEvMgrCtx();

	private:
		//当前manager的索引号，有些场景需要某个manager专门处理某种事件
		const int _idx;

		//退出循环的标志
		bool _quit;

		//线程池，可将函数放入其中异步执行
		ThreadPool* _pThreadPool;

		//循环器，在单独的一个线程中
		std::thread* _pLooper;

		//定时器，进行定时事件处理
		Timer* _pTimer;

		//保证eventSet的线程安全
		std::mutex _eventSetMutex;

		//保证timer线程安全
		std::mutex _timerMutex;

		//保证_actTimerTasks使用的线程安全
		std::mutex _timerQueMutex;

		//保证_removedEv事件列表的线程安全
		std::mutex _removedEvMutex;

		//保证设置上下文指针的线程安全
		std::mutex _ctxMutex;

		//被移除的事件列表，要移除某一个事件会先放在该列表中，一次循环结束才会真正delete
		std::vector<EventService*> _removedEv;

		//EventEpoller发现的活跃事件所放的列表
		std::vector<EventService*> _actEvServs;

		//Timer发现的超时事件所放的列表
		std::vector<std::function<void()>> _actTimerTasks;

		//活跃事件按照优先级所放的列表
		std::vector<EventService*> _priorityEvQue[EVENT_PRIORITY_TYPE_COUNT];

		//事件监视器
		EventEpoller _epoller;

		//事件集合
		std::set<EventService*> _eventSet;

		//EventManager区域唯一的上下文内容
		//考虑如下场景：
		//每个EventManager中所有的事件需要共享一个LRU缓冲区的时候而这个LRU队列
		//又是属于每个EventManager而非全局的，那么就需要设置这个上下文指针了。
		//EventManager不负责管理该对象的生命，默认为nullptr
		void* _pCtx;
	};

}
