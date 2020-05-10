//@Author Liu Yukang 
#pragma once
#include "EventService.h"
#include "Time.h"
#include "Timer.h"
#include "utils.h"

namespace kikilib
{
	//定时器事件服务
	//定时事件可读时，执行此刻所有需要执行的函数
	class TimerEventService : public EventService
	{
	public:
		TimerEventService(Socket& sock, EventManager* evMgr);
		TimerEventService(Socket&& sock, EventManager* evMgr);

		~TimerEventService() {}

		DISALLOW_COPY_MOVE_AND_ASSIGN(TimerEventService);

		void HandleReadEvent();

	};

}
