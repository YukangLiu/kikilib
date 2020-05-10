#pragma once
#include "Socket.h"

namespace kikilib
{
	class EventManager;
	class EventService;

	//事件服务生产器
	//职责：生产事件服务实例对象
	class EventServicePool
	{
	public:
		EventServicePool() {};
		virtual ~EventServicePool() {};

		//创建EventService对象实例的函数
		virtual EventService* CreateEventService(Socket& sock, EventManager* evMgr) = 0;

		virtual void RetrieveEventService(EventService* ev) = 0;
	};

}