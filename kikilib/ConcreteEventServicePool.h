//@Author Liu Yukang 
#pragma once
#include "EventServicePool.h"
#include "ObjPool.h"

namespace kikilib
{
	class EventManager;
	class EventService;

	//事件服务生产器
	//职责：生产事件服务实例对象
	template<class ConcreteEventService>
	class ConcreteEventServicePool : public EventServicePool
	{
	public:
		ConcreteEventServicePool() {};
		~ConcreteEventServicePool() {};

		//用户必须事件的创建EventService对象实例的函数
		EventService* CreateEventService(Socket& sock, EventManager* evMgr)
		{
			return _evServeConstructor.New(sock, evMgr);
		}

		void RetrieveEventService(EventService* ev)
		{
			_evServeConstructor.Delete(static_cast<void*>(ev));
		}

	private:
		ObjPool<ConcreteEventService> _evServeConstructor;
	};

}