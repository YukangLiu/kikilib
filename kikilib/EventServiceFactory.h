//@Author Liu Yukang 
#pragma once
#include "Socket.h"

namespace kikilib
{
	class EventManager;
	class EventService;

	//事件服务生产器
	//职责：生产事件服务实例对象
	template<class ConcreteEventService>
	class EventServiceFactory
	{
	public:
		EventServiceFactory() {};
		~EventServiceFactory() {};

		//用户必须事件的创建EventService对象实例的函数
		EventService* CreateEventService(Socket sock, EventManager* evMgr)
		{
			return new ConcreteEventService(sock, evMgr);
		}
	};

}