#pragma once

#include "Socket.h"
#include "ManagerSelector.h"
#include "utils.h"

#include <vector>
//#include <thread>

namespace kikilib
{
	class EventManager;
	class EventServiceFactory;
	class ThreadPool;

	//事件主宰者
	//职责：
	//1、创建管理所有的事件管理器
	//2、监听新到来的连接，创建新的事件服务实体，然后根据ManagerSelector的选择策略选择管理该新事件的管理器
	//3、创建线程池工具实体
	//需要管理以下类的生命：
	//1、线程池工具threadpool
	//2、事件管理器eventmanager
	class EventMaster
	{
	public:
		EventMaster(EventServiceFactory* pEvServeFac);

		~EventMaster();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventMaster);

		//创建多个EventManager线程然后循环accept
		//mgrCnt : 创建事件管理器的数量，一个事件管理器对应一个线程
		//listenPort : 要在哪个端口上循环listen
		void Loop(int mgrCnt, int listenPort);

		void Stop() { _stop = true; }

	private:

		bool _stop;

		Socket _listener;

		//std::thread* _acceptor;

		//创建事件服务的工厂
		EventServiceFactory* _pEvServeFac;

		//线程池
		ThreadPool* _pThreadPool;

		//事件管理器的选择器，用于选择下一个事件由哪个事件管理器管理
		ManagerSelector _mgrSelector;

		//事件管理器列表
		std::vector<EventManager*> _evMgrs;
	};

}
