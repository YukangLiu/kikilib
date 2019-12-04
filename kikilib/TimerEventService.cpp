//@Author Liu Yukang 
#include "TimerEventService.h"

using namespace kikilib;

TimerEventService::TimerEventService(Timer* timer, Socket sock, EventManager* evMgr)
	: EventService(sock, evMgr), _pTimer(timer)
{ }

void TimerEventService::HandleReadEvent()
{
	_pTimer->RunExpired();
}