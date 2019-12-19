//@Author Liu Yukang 
#pragma once
#include "EventService.h"
#include "Socket.h"
#include "EventManager.h"

#include <set>
#include <string>

class ChatRoomService;

extern std::set<ChatRoomService*> peersSet;

class ChatRoomService : public kikilib::EventService
{
public:
	ChatRoomService(kikilib::Socket sock, kikilib::EventManager* evMgr)
		: EventService(sock, evMgr) , _peerSets(&peersSet)
	{ }

	~ChatRoomService()
	{
		_peerSets->erase(this);
	}

	virtual void handleConnectionEvent();

	virtual void handleReadEvent();

	virtual void handleErrEvent();

private:
	std::set<ChatRoomService*>* _peerSets;

};
