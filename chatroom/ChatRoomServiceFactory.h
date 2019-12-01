#pragma once
#include "EventServiceFactory.h"
#include "ChatRoomService.h"

extern std::set<ChatRoomService*> peersSet;

class ChatRoomServiceFactory : public kikilib::EventServiceFactory
{

public:
	ChatRoomServiceFactory() {};
	virtual ~ChatRoomServiceFactory() {};

	virtual kikilib::EventService* CreateEventService(kikilib::Socket sock, kikilib::EventManager* evMgr)
	{
		return new ChatRoomService(&peersSet, sock, evMgr);
	}

};
