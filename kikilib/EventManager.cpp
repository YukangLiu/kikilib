//@Author Liu Yukang 
#include "EventManager.h"
#include "LogManager.h"
#include "EventService.h"
#include "TimerEventService.h"
#include "Timer.h"
#include "ThreadPool.h"

#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <signal.h>

using namespace kikilib;

EventManager::EventManager(int idx, ThreadPool* threadPool) 
	: _idx(idx), _quit(false), _pThreadPool(threadPool),  _pLooper(nullptr), _pTimer(nullptr)
{ }

EventManager::~EventManager()
{
	_quit = true;
	RecordLog(ERROR_DATA_INFORMATION, std::to_string(_idx) + " EventManager being deleted!");
	if (_pLooper)
	{
		_pLooper->join();
		delete _pLooper;
	}
	for (auto pEvServ : _eventSet)
	{
		delete pEvServ;
	}
	if (_pTimer)
	{
		delete _pTimer;
	}
}

bool EventManager::Loop()
{
	if (_pThreadPool == nullptr)
	{//判断线程池工具是否有效
		RecordLog(ERROR_DATA_INFORMATION, std::to_string(_idx) + " get a null threadpool!");
		return false;
	}
	//初始化EventEpoller
	if (!_epoller.Init())
	{
		RecordLog(ERROR_DATA_INFORMATION, std::to_string(_idx) + " init epoll fd failed!");
		return false;
	}
	//初始化定时器服务
	int timeFd = ::timerfd_create(CLOCK_MONOTONIC,
		TFD_NONBLOCK | TFD_CLOEXEC);
	if (timeFd < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION,std::to_string(_idx) + " eventManager timer init failed!");
		return false;
	}
	Socket timeSock(timeFd);
	_pTimer = new Timer(timeSock);
	EventService* pTimerServe = new TimerEventService(_pTimer, timeSock, this);
	//设置定时器事件为优先级最高的事件
	pTimerServe->SetEventPriority(IMMEDIATE_EVENT);

	Insert(pTimerServe);

	//初始化loop
	_pLooper = new std::thread(
		[this]
		{
			while (!this->_quit)
			{
				//清空所有列表
				if(this->_actEvServs.size())
                {
                    this->_actEvServs.clear();
                }
				for (int i = 0; i < EVENT_PRIORITY_TYPE_COUNT; ++i)
				{
				    if(this->_priorityEvQue[i].size())
                    {
                        this->_priorityEvQue[i].clear();
                    }
				}
				//获取活跃事件
				this->_epoller.GetActEvServ(Parameter::epollTimeOutMs, this->_actEvServs);
				//按优先级分队
				for (auto pEvServ : this->_actEvServs)
				{
					if (pEvServ->GetEventPriority() >= IMMEDIATE_EVENT)
					{
						(this->_priorityEvQue[IMMEDIATE_EVENT]).push_back(pEvServ);
					}
					else
					{
						(this->_priorityEvQue[NORMAL_EVENT]).push_back(pEvServ);
					}
				}
				//按照优先级处理事件
				for (int i = 0; i < EVENT_PRIORITY_TYPE_COUNT; ++i)
				{
					for (auto evServ : this->_priorityEvQue[i])
					{
						evServ->HandleEvent();
					}
				}
				//处理不再关注的事件
				for (auto unusedEv : this->_removedEv)
				{
					//从监听事件中移除
					this->_epoller.RemoveEv(unusedEv);
					//close这个fd
					delete unusedEv;
				}
				if(this->_removedEv.size())
                {
                    std::lock_guard<std::mutex> lock(_removedEvMutex);
                    this->_removedEv.clear();
                }
			}
		}
		);
	return true;
}

//向事件管理器中插入一个事件
void EventManager::Insert(EventService* ev)
{
	if (!ev)
	{
		return;
	}
	{
		std::lock_guard<std::mutex> lock(_eventSetMutex);
		_eventSet.insert(ev);
	}
	_epoller.AddEv(ev);
}

//向事件管理器中移除一个事件
void EventManager::Remove(EventService* ev)
{
	if (!ev)
	{
		return;
	}
	
	{//从映射表中删除事件
		std::lock_guard<std::mutex> lock(_eventSetMutex);
		auto it = _eventSet.find(ev);
		if (it != _eventSet.end())
		{
			_eventSet.erase(it);
		}
	}
	
	{//放入被移除事件列表
		std::lock_guard<std::mutex> lock(_removedEvMutex);
		_removedEv.push_back(ev);
	}
}

//向事件管理器中修改一个事件服务所关注的事件类型
void EventManager::Motify(EventService* ev)
{
	if (!ev)
	{
		return;
	}

	{
		std::lock_guard<std::mutex> lock(_eventSetMutex);
		if (_eventSet.find(ev) == _eventSet.end())
		{
			Insert(ev);
			return;
		}
	}
	_epoller.MotifyEv(ev);
}

//time时间后执行timerCb函数
void EventManager::RunAfter(Time time, std::function<void()>&& timerCb)
{
	Time runTime(Time::now().GetTimeVal() + time.GetTimeVal());

	_pTimer->RunAt(runTime, std::move(timerCb));
}

//time时间后执行timerCb函数
void EventManager::RunEvery(Time time, std::function<void()> timerCb)
{
	//这里lamda表达式不加括号会立刻执行，待测试
	std::function<void()> realTimerCb(
		[this, time, timerCb]
		{
			timerCb();
			this->RunEvery(time, timerCb);
		}
		);

	RunAfter(time, std::move(realTimerCb));

}

//每过time时间执行一次timerCb函数,直到isContinue函数返回false
void EventManager::RunEveryUntil(Time time, std::function<void()> timerCb, std::function<bool()> isContinue)
{
	std::function<void()> realTimerCb(
		[this, time, timerCb, isContinue]
		{
			if (isContinue())
			{
				timerCb();
				this->RunEveryUntil(time, timerCb, isContinue);
			}
		}
		);

	RunAfter(time, std::move(realTimerCb));
}

//将任务放在线程池中以达到异步执行的效果
void EventManager::RunInThreadPool(std::function<void()>&& func)
{
    return _pThreadPool->enqueue(std::move(func));
}
