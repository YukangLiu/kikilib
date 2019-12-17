//@Author Liu Yukang 
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

	virtual void handleReadEvent();

	virtual void handleErrEvent();

private:
	void sendUnImpletement();

	void sendNotFount();

	void sendHeader(std::string& path);

	void sendBody(FILE* fp);

	void sendFile(std::string& path);
};