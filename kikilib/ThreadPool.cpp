#include "ThreadPool.h"
#include "LogManager.h"
#include "Parameter.h"

using namespace kikilib;

// the constructor just launches some amount of workers
ThreadPool::ThreadPool()
	: _stop(false)
{
	_usableQue = 0;
	for (size_t i = 0; i < Parameter::threadPoolCnt; ++i)
	{
		_workers.emplace_back(
			[this]
			{
				while (true)
				{
					std::function<void()> task;

					{
						std::unique_lock<std::mutex> lock(this->_threadsMutex);
						this->_condition.wait(lock,
							[this] { return this->_stop || !this->_tasks[_usableQue].empty() || !this->_tasks[!_usableQue].empty(); });

						if (this->_stop && this->_tasks[_usableQue].empty() && this->_tasks[!_usableQue].empty())
						{
							return;
						}
						int taskQue = !_usableQue;
						if (this->_tasks[taskQue].empty())
						{//这里的目的是尽可能减少对主进程的阻塞
							this->_changeQueMutex.lock();
							_usableQue = taskQue;
							this->_changeQueMutex.unlock();
						}
						taskQue = !_usableQue;
						try
						{
							task = std::move(this->_tasks[taskQue].front());
							this->_tasks[taskQue].pop();
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
void ThreadPool::enqueue(std::function<void()>&& func)
{
	// don't allow enqueueing after stopping the pool
	if (_stop)
	{
		RecordLog(ERROR_DATA_INFORMATION, "unknown err in GetActServ()!");
		return;
	}

	{
		std::unique_lock<std::mutex> changeLock(_changeQueMutex);
		_tasks[_usableQue].emplace(std::move(func));
	}
	_condition.notify_one();
}

// the destructor joins all threads
ThreadPool::~ThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(_threadsMutex);
		_stop = true;
	}
	_condition.notify_all();
	for (std::thread& worker : _workers)
		worker.join();
}