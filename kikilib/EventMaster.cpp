#include "EventMaster.h"
#include "EventService.h"
#include "LogManager.h"
#include "Parameter.h"
#include "EventManager.h"
#include "EventServiceFactory.h"
#include "ThreadPool.h"

#include <signal.h>

using namespace kikilib;

EventMaster::EventMaster(EventServiceFactory* pEvServeFac)
	: _pEvServeFac(pEvServeFac), _stop(false)
{
	StartLogMgr(Parameter::logName);
	if (_listener.IsUseful())
	{
		_listener.SetTcpNoDelay(Parameter::isNoDelay);
		_listener.SetReuseAddr(true);
		_listener.SetReusePort(true);
		_listener.SetBlockSocket();
	}
	_pThreadPool = new ThreadPool();
}

EventMaster::~EventMaster()
{
	Stop();
	delete _pEvServeFac;
	delete _pThreadPool;
	EndLogMgr();
}


void EventMaster::Loop(int mgrCnt, int listenPort)
{
	if (!_listener.IsUseful())
	{
		RecordLog("listener unuseful!");
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
		_evMgrs.emplace_back(std::move(new EventManager(i)));
		_evMgrs.back()->Loop();
	}

	//一直accept，因为只有一个线程在accept，所以没有惊群问题
	while (!_stop)
	{
		Socket conn(_listener.Accept());
		if (!conn.IsUseful())
		{
			continue;
		}
		//RecordLog("accept a new usr!");
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