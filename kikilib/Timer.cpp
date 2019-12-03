#include "Timer.h"
#include "LogManager.h"

#include <sys/timerfd.h>
#include <string.h>

using namespace kikilib;

void Timer::RunExpired()
{
	Time nowTime = Time::now();

	{
		std::lock_guard<std::mutex> lock(_timerMutex);
		for (auto it = _timerCbMap.begin(); it != _timerCbMap.end() && it->first < nowTime; it = _timerCbMap.begin())
		{
			(it->second)();
			_timerCbMap.erase(it);
		}
	}
	
	if (!_timerCbMap.empty())
	{
		std::map<Time, std::function<void()>>::iterator it;

		{//不能锁RunAt，因为RunAt中也有同样的锁，会造成死锁
			std::lock_guard<std::mutex> lock(_timerMutex);
			it = _timerCbMap.begin();
		}
		
		RunAt(it->first, it->second);
	}
}

void Timer::RunAt(Time time, std::function<void()> cb)
{
	{
		std::lock_guard<std::mutex> lock(_timerMutex);
		_timerCbMap.insert(std::move(std::pair<Time, std::function<void()>>(time, cb)));
	}
	
	struct itimerspec newValue;
	struct itimerspec oldValue;
	memset(&newValue, 0, sizeof newValue);
	memset(&oldValue, 0, sizeof oldValue);
	newValue.it_value = time.TimeIntervalFromNow();
	int ret = ::timerfd_settime(_timeSock.fd(), 0, &newValue, &oldValue);
	if (ret)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("timerfd_settime failed. errno : ") + std::to_string(errno));
	}
}
