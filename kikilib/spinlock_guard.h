//@author Liu Yukang
#pragma once
#include <atomic>
#include "utils.h"

namespace kikilib {

	//配合std::atomic_int定义的二元信号量使用，为1表示资源可以使用，为0表示资源不可使用
	class SpinlockGuard
	{
	public:
		SpinlockGuard(std::atomic_int &sem)
			: sem_(sem)
		{
			int exp = 1;
			while (!sem_.compare_exchange_strong(exp, 0))
			{
				exp = 1;
			}
		}

		~SpinlockGuard()
		{
			sem_.store(1);
		}

		DISALLOW_COPY_MOVE_AND_ASSIGN(SpinlockGuard);

	private:
		std::atomic_int& sem_;

	};

}
