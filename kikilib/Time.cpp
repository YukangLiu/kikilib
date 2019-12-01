#include "Time.h"
#include <sys/time.h>

using namespace kikilib;

Time Time::now()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	int64_t seconds = tv.tv_sec;
	return Time(seconds * 1000 * 1000 + tv.tv_usec);
}

struct timespec Time::TimeIntervalFromNow()
{
	int64_t microseconds = _timeVal - Time::now().GetTimeVal();
	if (microseconds < 100)
	{
		microseconds = 100;
	}
	struct timespec ts;
	ts.tv_sec = static_cast<time_t>(
		microseconds / (1000 * 1000));
	ts.tv_nsec = static_cast<long>(
		(microseconds % (1000 * 1000)) * 1000);
	return ts;
}