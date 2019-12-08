//@Author Liu Yukang 
#pragma once

#include "Socket.h"
#include "ManagerSelector.h"
#include "utils.h"
#include "EventMaster.h"
#include "EventService.h"
#include "LogManager.h"
#include "Parameter.h"
#include "EventManager.h"
#include "EventServiceFactory.h"
#include "ThreadPool.h"

#include <fcntl.h>
#include <vector>
//#include <thread>

namespace kikilib
{
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
		EventMaster()
			: _stop(false)
		{
			StartLogMgr(Parameter::logName);
			if (_listener.IsUseful())
			{
				_listener.SetTcpNoDelay(Parameter::isNoDelay);
				_listener.SetReuseAddr(true);
				_listener.SetReusePort(true);
				_listener.SetBlockSocket();
			}
			_storedFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
			_pThreadPool = new ThreadPool();
		}

		~EventMaster()
		{
			Stop();
			if (_pThreadPool)
			{
				delete _pThreadPool;
			}
			if (_storedFd >= 0)
			{
				::close(_storedFd);
			}
			EndLogMgr();
		}

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventMaster);

		//创建多个EventManager线程然后循环accept
		//mgrCnt : 创建事件管理器的数量，一个事件管理器对应一个线程
		//listenPort : 要在哪个端口上循环listen
		void Loop(int mgrCnt, int listenPort)
		{
			if (!_listener.IsUseful())
			{
				RecordLog("listener unuseful!");
				return;
			}

			if (_storedFd < 0)
			{
				RecordLog("_storedFd unuseful!");
				return;
			}

			if (_listener.Bind(listenPort) < 0)
			{
				return;
			}
			_listener.Listen();

			_mgrSelector.SetManagerCnt(mgrCnt);

			for (int i = 0; i < mgrCnt; ++i)
			{
				_evMgrs.emplace_back(std::move(new EventManager(i, _pThreadPool)));
				if (!_evMgrs.back()->Loop())
				{
					return;
					RecordLog("eventManager loop failed!");
				}
			}

			//一直accept，因为只有一个线程在accept，所以没有惊群问题
			while (!_stop)
			{
				Socket conn(_listener.Accept());
				if (!conn.IsUseful())
				{
					//if (errno == EMFILE)
					//{
					//	::close(_storedFd);
					//	_storedFd = ::accept(_listener.fd(), NULL, NULL);
					//	::close(_storedFd);
					//	_storedFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
					//}

					//if (_storedFd < 0)
					//{
					//	RecordLog("_storedFd unuseful after give a new connection!");
					//	break;
					//}
					continue;
				}
				conn.SetTcpNoDelay(Parameter::isNoDelay);
				RecordLog("accept a new usr ,ip : " + conn.GetIp());
				int nextMgrIdx = _mgrSelector.Next();
				EventService* ev = _pEvServeFac.CreateEventService(conn, _evMgrs[nextMgrIdx]);
				if (ev)
				{
					ev->HandleConnectionEvent();
					if (ev->IsConnected())
					{
						_evMgrs[nextMgrIdx]->Insert(ev);
					}
				}
			}

			//关闭所有evMgr中的服务
			for (auto evMgr : _evMgrs)
			{
				delete evMgr;
			}
		}

		void Stop() { _stop = true; }

	private:

		bool _stop;

		//std::thread* _acceptor;

		//创建事件服务的工厂
		EventServiceFactory<ConcreteEventService> _pEvServeFac;

		//线程池
		ThreadPool* _pThreadPool;

		//用于监听的socket
		Socket _listener;

		//事件管理器的选择器，用于选择下一个事件由哪个事件管理器管理
		ManagerSelector _mgrSelector;

		//事件管理器列表
		std::vector<EventManager*> _evMgrs;

		int _storedFd;
	};


}
