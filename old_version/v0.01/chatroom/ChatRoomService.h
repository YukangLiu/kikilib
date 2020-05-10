//@Author Liu Yukang 
#pragma once
#include "EventService.h"
#include "Socket.h"
#include "EventManager.h"

#include <set>
#include <string>

class ChatRoomService : public kikilib::EventService
{
public:
	ChatRoomService(std::set<ChatRoomService*>* peerSets, kikilib::Socket sock, kikilib::EventManager* evMgr)
		: _peerSets(peerSets), EventService(sock, evMgr)
	{ }

	~ChatRoomService()
	{
		_peerSets->erase(this);
	}

	virtual void HandleConnectionEvent();

	virtual void HandleReadEvent();

	virtual void HandleErrEvent();

private:
	std::set<ChatRoomService*>* _peerSets;

};
