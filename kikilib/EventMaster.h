//@Author Liu Yukang 
#pragma once

#include "Socket.h"
#include "ConcreteEventServicePool.h"
#include "ManagerSelector.h"
#include "EventManager.h"

#include "ThreadPool.h"
#include "LogManager.h"

#include "utils.h"
#include "Parameter.h"

//#include <fcntl.h>
#include <vector>
//#include <thread>

namespace kikilib
{
	class EventService;

	//事件主宰者
	//职责：
	//1、创建管理所有的事件管理器
	//2、监听新到来的连接，创建新的事件服务实体，然后根据ManagerSelector的选择策略选择管理该新事件的管理器
	//3、创建线程池工具实体
	//需要管理以下类的生命：
	//1、线程池工具threadpool
	//2、事件管理器eventmanager
	//3、事件服务工厂eventservicefactory
	template<class ConcreteEventService>
	class EventMaster
	{
	public:
		EventMaster();

		~EventMaster();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventMaster);

		//创建多个EventManager线程
		//mgrCnt : 创建事件管理器的数量，一个事件管理器对应一个线程
		//listenPort : 要在哪个端口上循环listen
		bool init(int mgrCnt, int listenPort);

		//设置EventManager区域唯一的上下文内容
		//idx为要设置几号EventManager的上下文内容
		//ctx为要设置的EventManager上下文内容
		//需求来源，考虑如下场景：
		//每个EventManager中所有的事件需要共享一个LRU缓冲区的时候而这个LRU队列
		//又是属于每个EventManager而非全局的，那么就需要设置这个上下文指针了。
		//EventManager不负责管理该对象的生命，默认为nullptr
		bool setEvMgrCtx(int idx, void* ctx);

		//循环accept
		void loop();

		void stop() { _stop = true; }

	private:

		bool _stop;

		//std::thread* _acceptor;

		//线程池
		ThreadPool* _pThreadPool;

		//用于监听的socket
		Socket _listener;

		//事件管理器的选择器，用于选择下一个事件由哪个事件管理器管理
		ManagerSelector* _pMgrSelector;

		//事件管理器列表
		std::vector<EventManager*> _evMgrs;
	};

	template<class ConcreteEventService>
	inline EventMaster<ConcreteEventService>::EventMaster()
		: _stop(true), _pThreadPool(nullptr), _pMgrSelector(nullptr)
	{
		StartLogMgr(Parameter::logName);
		_pThreadPool = new ThreadPool();
		_pMgrSelector = new ManagerSelector(_evMgrs);
	}

	template<class ConcreteEventService>
	inline EventMaster<ConcreteEventService>::~EventMaster()
	{
		stop();
		if (_pThreadPool)
		{
			delete _pThreadPool;
		}
		if (_pMgrSelector)
		{
			delete _pMgrSelector;
		}
		EndLogMgr();
	}

	//创建多个EventManager线程
	//mgrCnt : 创建事件管理器的数量，一个事件管理器对应一个线程
	//listenPort : 要在哪个端口上循环listen
	template<class ConcreteEventService>
	inline bool EventMaster<ConcreteEventService>::init(int mgrCnt, int listenPort)
	{
		//初始化监听套接字
		if (_listener.isUseful())
		{
			_listener.setTcpNoDelay(Parameter::isNoDelay);
			_listener.setReuseAddr(true);
			_listener.setReusePort(true);
			_listener.setBlockSocket();
			if (_listener.bind(listenPort) < 0)
			{
				return false;
			}
			_listener.listen();
		}
		else
		{
			RecordLog("listener unuseful!");
			return false;
		}

		//初始化EventManager
		for (int i = 0; i < mgrCnt; ++i)
		{
			EventServicePool* pool = new ConcreteEventServicePool<ConcreteEventService>();
			_evMgrs.emplace_back(std::move(new EventManager(i, pool, _pThreadPool)));
			if (!_evMgrs.back()->loop())
			{
				return false;
				RecordLog("eventManager loop failed!");
			}
		}

		_stop = false;
		return true;
	}

	//设置EventManager区域唯一的上下文内容
	//idx为要设置几号EventManager的上下文内容
	//ctx为要设置的EventManager上下文内容
	template<class ConcreteEventService>
	inline bool EventMaster<ConcreteEventService>::setEvMgrCtx(int idx, void* ctx)
	{
		if (idx >= _evMgrs.size() || idx < 0)
		{
			return false;
		}
		_evMgrs[idx]->setEvMgrCtx(ctx);
		return true;
	}

	//循环accept
	template<class ConcreteEventService>
	inline void EventMaster<ConcreteEventService>::loop()
	{
		//一直accept，因为只有一个线程在accept，所以没有惊群问题
		while (!_stop)
		{
			Socket conn(_listener.accept());
			if (!conn.isUseful() || conn.fd() > Parameter::maxEventServiceCnt)
			{
				continue;
			}

			conn.setTcpNoDelay(Parameter::isNoDelay);
			RecordLog("accept a new usr ,ip : " + conn.ip());
			EventManager* pEvMgr = _pMgrSelector->next();
			EventService* ev = pEvMgr->CreateEventService(conn);
			if (ev)
			{
				pEvMgr->insertEv(ev);
			}
			else
			{
				RecordLog("create an eventservice failed!");
			}
		}

		//关闭所有evMgr中的服务
		for (auto evMgr : _evMgrs)
		{
			delete evMgr;
		}
	}
}
