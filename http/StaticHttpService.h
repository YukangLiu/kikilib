#pragma once
#include "EventService.h"
#include "Socket.h"
#include "EventManager.h"

class StaticHttpService : public kikilib::EventService
{
public:
	StaticHttpService(kikilib::Socket sock, kikilib::EventManager* evMgr)
		: EventService(sock, evMgr)
	{ };

	~StaticHttpService()
	{ };

	virtual void HandleReadEvent();

	virtual void HandleErrEvent();

private:
	void SendUnImpletement();

	void SendNotFount();

	void SendHeader(std::string& path);

	void SendBody(FILE* fp);

	void SendFile(std::string& path);
};