//@Author Liu Yukang 
#pragma once

#include <stdint.h>

struct timespec;

namespace kikilib
{
	//时间类，单位：微秒us
	class Time
	{
	public:
		Time(int64_t msSinceEpoch) : _timeVal(msSinceEpoch) {}

		Time(const Time& time) { _timeVal = time._timeVal; }

		Time(const Time&& time) { _timeVal = time._timeVal; }

		Time& operator=(const Time& time)
		{
			_timeVal = time._timeVal;
			return *this;
		}

		~Time() {}

		//1970年1月1日到现在的微秒数
		static Time now();

		//到现在的时间
		struct timespec TimeIntervalFromNow();

		int64_t GetTimeVal() { return _timeVal; }

	private:
		int64_t _timeVal;

	};

	inline bool operator < (Time lhs, Time rhs)
	{
		return lhs.GetTimeVal() < rhs.GetTimeVal();
	}

	inline bool operator <= (Time lhs, Time rhs)
	{
		return lhs.GetTimeVal() <= rhs.GetTimeVal();
	}

	inline bool operator > (Time lhs, Time rhs)
	{
		return lhs.GetTimeVal() > rhs.GetTimeVal();
	}

	inline bool operator >= (Time lhs, Time rhs)
	{
		return lhs.GetTimeVal() >= rhs.GetTimeVal();
	}

	inline bool operator == (Time lhs, Time rhs)
	{
		return lhs.GetTimeVal() == rhs.GetTimeVal();
	}

}
