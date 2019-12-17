//@Author Liu Yukang 
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

	virtual void handleReadEvent();

	virtual void handleErrEvent();

	int getVal() { return _valForPool; }

	void modifyVal() { _valForPool = 100; }

private:

	int _valForPool;
};
