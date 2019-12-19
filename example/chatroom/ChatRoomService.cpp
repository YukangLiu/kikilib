//@Author Liu Yukang 
#include "ChatRoomService.h"
#include "LogManager.h"

void ChatRoomService::handleConnectionEvent()
{
	RecordLog("accept a new client(ip:port) " + peerIP() + " : " + std::to_string(peerPort()));
	_peerSets->insert(this);
};

void ChatRoomService::handleReadEvent()
{
	std::string str = readAll();
	for (auto peer : *_peerSets)
	{
		peer->sendContent(str);
	}
};

void ChatRoomService::handleErrEvent()
{
    _peerSets->erase(this);
	forceClose();
};