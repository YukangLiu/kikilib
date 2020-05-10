//@Author Liu Yukang 
#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <stdexcept>

#include "utils.h"

namespace kikilib
{
	//线程池
	//双任务队列，一条一直被生产者加入任务，一条一直在被线程池中的线程消费执行任务
	class ThreadPool {
	public:
		ThreadPool();
		~ThreadPool();

		//将函数入队
		void enqueue(std::function<void()>&&);
		
		//不允许复制
		DISALLOW_COPY_MOVE_AND_ASSIGN(ThreadPool);

	private:
		//当前可以向其中加入任务的队列下标
		int _usableQue;
		bool _stop;

		// need to keep track of threads so we can join them
		std::vector< std::thread > _workers;
		// 双任务队列，一条一直在被外界放任务，一条一直在被线程消费执行任务
		std::queue< std::function<void()> > _tasks[2];

		// synchronization
		std::mutex _threadsMutex;//执行任务的线程之间的锁
		std::mutex _changeQueMutex;//若需要切换任务队列，使用此锁
		std::condition_variable _condition;
	};
}