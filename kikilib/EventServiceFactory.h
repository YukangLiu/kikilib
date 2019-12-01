#pragma once
#include "Socket.h"

namespace kikilib
{
	class EventManager;
	class EventService;

	//事件服务生产器
	//职责：生产事件服务实例对象
	//用法：
	//用户继承EventServiceFactory类，实现一个创建EventService对象的方法
	//   然后将工厂类实例传给EventMaster，服务器即可运转，每有一个
	//   新的连接到来，EventMaster就会使用工厂创建一个该事件服务对象为
	//   新的连接服务
	class EventServiceFactory
	{
	public:
		EventServiceFactory() {};
		virtual ~EventServiceFactory() {};

		//用户必须事件的创建EventService对象实例的函数
		virtual EventService* CreateEventService(Socket sock, EventManager* evMgr) = 0;
	};

}