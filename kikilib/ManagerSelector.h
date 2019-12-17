//@Author Liu Yukang 
#pragma once
#include <vector>

namespace kikilib
{
	class EventManager;

	enum scheduleStrategy
	{
		MIN_EVENT_FIRST = 0 , //最少事件优先
		ROUND_ROBIN			  //轮流分发
	};

	//事件管理器选择器，决定下一个事件应该放入哪个事件管理器中
	class ManagerSelector
	{
	public:
		ManagerSelector(std::vector<EventManager*>& evMgrs, int strategy = MIN_EVENT_FIRST) :  _curMgr(-1) , _strategy(strategy) , _evMgrs(evMgrs) {}
		~ManagerSelector() {}

		//设置分发任务的策略
		//MIN_EVENT_FIRST则每次挑选EventService最少的EventManager接收新连接
		//ROUND_ROBIN则每次轮流挑选EventManager接收新连接
		void setStrategy(int strategy);

		EventManager* next();

	private:
		int _curMgr;

		int _strategy;

		std::vector<EventManager*>& _evMgrs;

	};

}