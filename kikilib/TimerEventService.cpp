#include "TimerEventService.h"

using namespace kikilib;

TimerEventService::TimerEventService(Timer* timer, Socket sock, EventManager* evMgr)
	: _pTimer(timer), EventService(sock, evMgr)
{ }

void TimerEventService::HandleReadEvent()
{
	_pTimer->RunExpired();
}