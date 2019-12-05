//@Author Liu Yukang 
#include "EventMaster.h"
#include "EventService.h"
#include "LogManager.h"
#include "Parameter.h"
#include "EventManager.h"
#include "EventServiceFactory.h"
#include "ThreadPool.h"

#include <fcntl.h>

using namespace kikilib;

EventMaster::EventMaster(EventServiceFactory* pEvServeFac)
	: _stop(false), _pEvServeFac(pEvServeFac)
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

EventMaster::~EventMaster()
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


void EventMaster::Loop(int mgrCnt, int listenPort)
{
	if (!_listener.IsUseful())
	{
		RecordLog("listener unuseful!");
		return;
	}

	if (_storedFd  < 0)
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
		RecordLog("accept a new usr ,fd : " + std::to_string(conn.fd()));
		int nextMgrIdx = _mgrSelector.Next();
		EventService* ev = _pEvServeFac->CreateEventService(conn, _evMgrs[nextMgrIdx]);
		if (ev)
		{
			ev->HandleConnectionEvent();
			if (ev->IsConnected())
			{
				_evMgrs[nextMgrIdx]->Insert(ev);
			}
			else
			{
				delete ev;
			}
		}
	}

	//关闭所有evMgr中的服务
	for (auto evMgr : _evMgrs)
	{
		delete evMgr;
	}
}