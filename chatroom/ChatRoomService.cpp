#include "ChatRoomService.h"
#include "LogManager.h"

void ChatRoomService::HandleConnectionEvent()
{
	RecordLog("accept a new client(ip:port) " + peerIP() + " : " + std::to_string(peerPort()));
	_peerSets->insert(this);
};

void ChatRoomService::HandleReadEvent()
{
	std::string str = ReadAll();
	for (auto peer : *_peerSets)
	{
		peer->WriteBuf(str);
	}
};

void ChatRoomService::HandleErrEvent()
{
    _peerSets->erase(this);
	Close();
};