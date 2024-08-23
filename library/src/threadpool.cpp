#include "threadpool.hpp"

#include "platform.hpp"

#if defined(PLATFORM_WINDOWS)
#include <windows.h>
#endif

static bool set_affinity(std::thread & thread, size_t i);

// Initialize the threadpool with a certain size
ThreadPool::ThreadPool(std::size_t size, std::size_t max_batch):
	finish(false),
	num_workers(0)
{
	if (max_batch == 0)
		max_batch = size;
	// Create threads
	for (decltype(size) i = 0; i < size; ++i)
	{
		workers.emplace_back([this, size, max_batch]()
		{
			// Keep data safe by locking
			std::unique_lock<std::mutex> lock(task_mutex);

			do
			{
				if (!tasks.empty())
				{
					// Calculate maximum amount of tasks to pop. Should ensure
					// that there is enough tasks for all workers.
					auto max_pop = (std::min)((tasks.size() + size - 1) / size, max_batch);
					std::vector<std::function<void()>> batch;
					batch.reserve(max_pop);
					for (decltype(max_pop) i = 0; i < max_pop; ++i)
					{
						batch.emplace_back(std::move(tasks.top().second));
						tasks.pop();
					}
					++num_workers;
					lock.unlock();

					for (auto & task : batch)
						task();

					lock.lock();
					--num_workers;
					idle_cond.notify_all();
				}
				else
				{
					task_cond.wait(lock);
				}
			}
			while (!finish || !tasks.empty());

			lock.unlock();
			task_cond.notify_all();
		});
		set_affinity(workers.back(), i);
	}
}

// Close down pool, waiting for everything to finalize
ThreadPool::~ThreadPool()
{
	{
		std::lock_guard<std::mutex> guard(task_mutex);
		finish = true;
	}
	task_cond.notify_all();

	// Wait for all threads to close down
	for (auto & worker : workers)
	{
		worker.join();
	}
}

threadpool::Transaction ThreadPool::begin_transaction()
{
	return threadpool::Transaction();
}

void ThreadPool::commit(threadpool::Transaction & trans)
{
	auto & pretrans = trans.transaction;
	if (pretrans.empty())
		return;
	decltype(tasks) transaction;
	std::swap(transaction, pretrans);
	std::unique_lock<std::mutex> lock(task_mutex);
	if (transaction.size() > tasks.size())
		std::swap(transaction, tasks);
	while (!transaction.empty())
	{
		tasks.push(std::move(transaction.top()));
		transaction.pop();
	}
	task_cond.notify_all();
}

// Check if pool is idling
bool ThreadPool::idle()
{
	std::unique_lock<std::mutex> guard(task_mutex);
	return idleUnsafe();
}

// Wait for pool to finish its work and become idle
void ThreadPool::wait()
{
	std::unique_lock<std::mutex> guard(task_mutex);
	if (idleUnsafe())
		return;
	idle_cond.wait(guard, std::bind(&ThreadPool::idleUnsafe, this));
}

// Unsafe check for checking if pool is idling
inline bool ThreadPool::idleUnsafe()
{
	return num_workers == 0 && tasks.empty();
}

#define ENABLE_AFFINITY 0

static bool set_affinity(std::thread & thread, size_t i)
{
#if ENABLE_AFFINITY
#if	defined(PLATFORM_WINDOWS)
	auto handle = thread.native_handle();
	int mask = 1 << i;
	return SetThreadAffinityMask(handle, mask) != 0;
#elif defined(PLATFORM_UNIX)
	pthread_t handle = thread.native_handle();
	cpu_set_t cpuset;
	CPU_ZERO(&cpuset);
	CPU_SET(i, &cpuset);
	return pthread_setaffinity_np(handle, sizeof(cpuset), &cpuset) != 0;
#else
	return false;
#endif
#else
	return false;
#endif
}
