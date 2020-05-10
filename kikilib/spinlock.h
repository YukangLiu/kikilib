//@Author Liu Yukang 
#pragma once
#include <atomic>
#include "utils.h"

namespace kikilib {

	//配合std::atomic_int定义的二元信号量使用，为1表示资源可以使用，为0表示资源不可使用
	class Spinlock
	{
	public:
		Spinlock()
			: sem_(1)
		{ }

		~Spinlock() { unlock(); }

		DISALLOW_COPY_MOVE_AND_ASSIGN(Spinlock);

		void lock()
		{
			int exp = 1;
			while (!sem_.compare_exchange_strong(exp, 0))
			{
				exp = 1;
			}
		}

		void unlock()
		{
			sem_.store(1);
		}

	private:
		std::atomic_int sem_;

	};

}