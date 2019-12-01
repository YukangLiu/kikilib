#pragma once
#include "EventServiceFactory.h"
#include "StaticHttpService.h"

class StaticHttpServiceFactory : public kikilib::EventServiceFactory
{

public:
	StaticHttpServiceFactory() {};
	virtual ~StaticHttpServiceFactory() {};

	virtual kikilib::EventService* CreateEventService(kikilib::Socket sock, kikilib::EventManager* evMgr)
	{
		return new StaticHttpService(sock, evMgr);
	}

};
