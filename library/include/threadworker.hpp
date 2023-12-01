#pragma once
#ifndef THREADWORKER_HPP
#define THREADWORKER_HPP

#include "any.hpp"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <tuple>
#include <functional>

class ThreadWorker
{
public:

	~ThreadWorker();

	// Start a worker
	void start(std::function<void(Any)> && func);

	// Send data to worker
	void enqueue(Any datum);

	// Stop worker
	void stop();

	// Wait for worker
	void wait();

	// Check if worker is idle
	bool idle();

private:
	std::thread thread;
	std::mutex mutex;
	std::condition_variable thread_cond;
	std::condition_variable finish_cond;
	bool finish = false;
	int working = 0;
	std::queue<Any> data;
};

#endif // THREADWORKER_HPP
