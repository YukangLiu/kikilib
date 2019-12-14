//@Author Liu Yukang 
#include "Time.h"
#include <sys/time.h>

using namespace kikilib;

//volatile int64_t Time::_roughTime = 0;

Time Time::now()
{
	struct timeval tv;
	::gettimeofday(&tv, 0);
	int64_t seconds = tv.tv_sec;
	//每次调用now都会更新服务器大致时间，因为是大致时间，存在竞态没有关系
	//_roughTime = seconds;
	return Time(seconds * 1000 * 1000 + tv.tv_usec);
}

time_t Time::nowSec()
{
	struct timeval tv;
	::gettimeofday(&tv, 0);
	return tv.tv_sec;
}

//void Time::UpdataRoughTime()
//{//因为是大致时间，存在竞态没有关系
//	struct timeval tv;
//	gettimeofday(&tv, 0);
//	_roughTime = tv.tv_sec;
//}

void Time::ToLocalTime(time_t second, long timezone, struct tm* tm_time)
{
	uint32_t n32_Pass4year;
	int32_t n32_hpery;

	//计算时差
	second = second - timezone;

	if (second < 0)
	{
		second = 0;
	}
	//取秒时间
	tm_time->tm_sec = (int)(second % 60);
	second /= 60;
	//取分钟时间
	tm_time->tm_min = (int)(second % 60);
	second /= 60;
	//取过去多少个四年，每四年有 1461*24 小时
	n32_Pass4year = ((unsigned int)second / (1461L * 24L));
	//计算年份
	tm_time->tm_year = (n32_Pass4year << 2) + 70;
	//四年中剩下的小时数
	second %= 1461L * 24L;
	//校正闰年影响的年份，计算一年中剩下的小时数
	for (;;)
	{
		//一年的小时数
		n32_hpery = 365 * 24;
		//判断闰年
		if ((tm_time->tm_year & 3) == 0)
		{
			//是闰年，一年则多24小时，即一天
			n32_hpery += 24;
		}
		if (second < n32_hpery)
		{
			break;
		}
		tm_time->tm_year++;
		second -= n32_hpery;
	}
	//小时数
	tm_time->tm_hour = (int)(second % 24);
	//一年中剩下的天数
	second /= 24;
	//假定为闰年
	second++;
	//校正润年的误差，计算月份，日期
	if ((tm_time->tm_year & 3) == 0)
	{
		if (second > 60)
		{
			second--;
		}
		else
		{
			if (second == 60)
			{
				tm_time->tm_mon = 1;
				tm_time->tm_mday = 29;
				return;
			}
		}
	}
	//计算月日
	for (tm_time->tm_mon = 0; Days[tm_time->tm_mon] < second; tm_time->tm_mon++)
	{
		second -= Days[tm_time->tm_mon];
	}

	tm_time->tm_mday = (int)(second);

	return;
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