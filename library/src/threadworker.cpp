#include "threadworker.hpp"

// RAII to avoid it still running
ThreadWorker::~ThreadWorker()
{
	stop();
}

// Start worker with function
void ThreadWorker::start(std::function<void(Any)> && func)
{
	// Verify thread exists
	if (thread.joinable())
		return;
	finish = false;

	thread = std::thread([this, func]()
	{
		std::unique_lock<std::mutex> lock(mutex);
		do
		{
			if (!data.empty())
			{
				auto datum = data.front();
				data.pop();
				++working;
				lock.unlock();

				func(datum);

				lock.lock();
				--working;
			}
			else
			{
				finish_cond.notify_all();
				thread_cond.wait(lock);
			}
		}
		while (!finish);
	});
}

// Add data to worker
void ThreadWorker::enqueue(Any datum)
{
	{
		std::lock_guard<std::mutex> guard(mutex);
		data.push(datum);
	}
	thread_cond.notify_one();
}

// Stop the worker
void ThreadWorker::stop()
{
	{
		std::lock_guard<std::mutex> guard(mutex);
		finish = true;
	}
	thread_cond.notify_all();

	thread.join();
}

// Wait for the worker to finish
void ThreadWorker::wait()
{
	std::unique_lock<std::mutex> guard(mutex);
	// Avoid deadlock, checking for work
	if (working == 0 && data.empty())
		return;
	finish_cond.wait(guard);
}

// Check if the worker is idle
bool ThreadWorker::idle()
{
	std::lock_guard<std::mutex> guard(mutex);
	return working == 0 && data.empty();
}
