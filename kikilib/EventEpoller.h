#pragma once
#include "utils.h"

#include <vector>

struct epoll_event;

namespace kikilib
{
	class EventService;

	//事件监视器
	//职责：
	//1、监视发现当前被触发的事件
	//2、修改被监视的事件
	class EventEpoller
	{
	public:
		EventEpoller();
		~EventEpoller();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventEpoller);

		//修改EventEpoller中的事件
		void MotifyEv(EventService* evServ);

		//向EventEpoller中添加事件
		void AddEv(EventService* evServ);

		//从EventEpoller中移除事件
		void RemoveEv(EventService* evServ);

		//获取被激活的事件服务
		void GetActEvServ(int timeOutMs, std::vector<EventService*>& activeEvServs);

	private:

		int _epollFd;
		std::vector<struct epoll_event> _activeEpollEvents;

	};

}
