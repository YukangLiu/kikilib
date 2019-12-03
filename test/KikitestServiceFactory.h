#pragma once
#include "EventServiceFactory.h"
#include "KikitestService.h"

class KikitestServiceFactory : public kikilib::EventServiceFactory
{

public:
	KikitestServiceFactory() {};
	virtual ~KikitestServiceFactory() {};

	virtual kikilib::EventService* CreateEventService(kikilib::Socket sock, kikilib::EventManager* evMgr)
	{
		return new KikitestService(sock, evMgr);
	}

};

