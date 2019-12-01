#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <signal.h>

#include "LogManager.h"
#include "Parameter.h"
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
		template<class F, class... Args>
		auto enqueue(F&& f, Args&&... args)
			->std::future<typename std::result_of<F(Args...)>::type>;
		
		//不允许复制
		DISALLOW_COPY_MOVE_AND_ASSIGN(ThreadPool);

	private:
		// need to keep track of threads so we can join them
		std::vector< std::thread > workers;
		// 双任务队列，一条一直在被外界放任务，一条一直在被线程消费执行任务
		std::queue< std::function<void()> > tasks[2];

		// synchronization
		std::mutex _threadsMutex;//执行任务的线程之间的锁
		std::mutex _changeQueMutex;//若需要切换任务队列，使用此锁
		std::condition_variable condition;

		//当前可以向其中加入任务的队列下标
		int _usableQue;
		bool stop;
	};

	// the constructor just launches some amount of workers
	inline ThreadPool::ThreadPool()
		: stop(false)
	{
		_usableQue = 0;
		for (size_t i = 0; i < Parameter::threadPoolCnt; ++i)
		{
			workers.emplace_back(
				[this]
				{
					while (true)
					{
						std::function<void()> task;

						{
							std::unique_lock<std::mutex> lock(this->_threadsMutex);
							this->condition.wait(lock,
								[this] { return this->stop || !this->tasks[_usableQue].empty() || !this->tasks[!_usableQue].empty(); });

							if (this->stop && this->tasks[_usableQue].empty() && this->tasks[!_usableQue].empty())
							{
								return;
							}
							int taskQue = !_usableQue;
							if (this->tasks[taskQue].empty())
							{//这里的目的是尽可能减少对主进程的阻塞
								this->_changeQueMutex.lock();
								_usableQue = taskQue;
								this->_changeQueMutex.unlock();
							}
							taskQue = !_usableQue;
							try
							{
								task = std::move(this->tasks[taskQue].front());
								this->tasks[taskQue].pop();
							}
							catch (std::exception & e)
							{
								RecordLog(e.what());
							}

						}
						try
						{
							task();
						}
                        catch (std::exception & e)
                        {
                            RecordLog(e.what());
                        }

					}
				});
		}
	}

	// add new work item to the pool
	template<class F, class... Args>
	auto ThreadPool::enqueue(F&& f, Args&&... args)
		-> std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared< std::packaged_task<return_type()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();

		// don't allow enqueueing after stopping the pool
		if (stop)
        {
            RecordLog(ERROR_DATA_INFORMATION, "unknown err in GetActServ()!");
            throw("error : unknown err in GetActServ()!");
        }

		{
			std::unique_lock<std::mutex> changeLock(_changeQueMutex);
			tasks[_usableQue].emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

	// the destructor joins all threads
	inline ThreadPool::~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(_threadsMutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread& worker : workers)
			worker.join();
	}

}