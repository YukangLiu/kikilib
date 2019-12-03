#pragma once
#include "EventService.h"
#include "Socket.h"
#include "EventManager.h"

class KikitestService : public kikilib::EventService
{
public:
	KikitestService(kikilib::Socket sock, kikilib::EventManager* evMgr)
		: EventService(sock, evMgr), _valForPool(0)
	{ }

	~KikitestService()
	{ }

	virtual void HandleReadEvent();

	virtual void HandleErrEvent();

	int GetVal() { return _valForPool; }

	void MotifyVal() { _valForPool = 100; }

private:

	int _valForPool;
};
