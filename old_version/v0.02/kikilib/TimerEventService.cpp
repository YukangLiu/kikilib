//@Author Liu Yukang 
#include "TimerEventService.h"

using namespace kikilib;

TimerEventService::TimerEventService(Socket& sock, EventManager* evMgr)
	: EventService(sock, evMgr)
{ }

TimerEventService::TimerEventService(Socket&& sock, EventManager* evMgr)
	: EventService(std::move(sock), evMgr)
{ }

void TimerEventService::HandleReadEvent()
{
	ReadAll();
	RunExpired();
}