#include "Timer.h"
#include "LogManager.h"

#include <sys/timerfd.h>
#include <string.h>

using namespace kikilib;

void Timer::RunExpired()
{
	Time nowTime = Time::now();
	for (auto it = _timerCbMap.begin(); it != _timerCbMap.end() && it->first < nowTime; it = _timerCbMap.begin())
	{
		(it->second)();
		_timerCbMap.erase(it);
	}
	if (!_timerCbMap.empty())
	{
		auto it = _timerCbMap.begin();
		RunAt(it->first, it->second);
	}
}

void Timer::RunAt(Time time, std::function<void()> cb)
{
	_timerCbMap.insert(std::move(std::pair<Time, std::function<void()>>(time, cb)));
	struct itimerspec newValue;
	struct itimerspec oldValue;
	memset(&newValue, 0, sizeof newValue);
	memset(&oldValue, 0, sizeof oldValue);
	newValue.it_value = time.TimeIntervalFromNow();
	int ret = ::timerfd_settime(_timeSock.fd(), 0, &newValue, &oldValue);
	if (ret)
	{
		RecordLog(ERROR_DATA_INFORMATION, "timerfd_settime");
	}
}
