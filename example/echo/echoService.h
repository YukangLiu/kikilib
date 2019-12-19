//@Author Liu Yukang 
#pragma once
#include "EventService.h"
#include "EventManager.h"

class EchoService : public kikilib::EventService
{
public:
	EchoService(kikilib::Socket sock, kikilib::EventManager* evMgr)
		: EventService(sock, evMgr)
	{ };

	~EchoService()
	{ };

	virtual void handleReadEvent();

	virtual void handleErrEvent();

private:
};