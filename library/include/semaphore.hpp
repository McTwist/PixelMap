#pragma once
#ifndef SEMAPHORE_HPP
#define SEMAPHORE_HPP

#include <mutex>
#include <condition_variable>

// TODO: Replace with std::counting_semaphore (C++20)

/**
 * @brief A basic semaphore
 * @tparam M A mutex locking mechanism for the condition
 * @tparam C A condidional variable to lock the thread
 * Basic semaphore implementation to make sure only a certain amount of
 * threads can reach a resource.
 */
template<class M, class C>
class basic_semaphore
{
public:
	basic_semaphore(std::size_t _count) : count(_count) {}

	/**
	 * @brief Notify the semaphore for being done
	 * Sends a signal to all threads currently waiting.
	 */
	inline void notify()
	{
		std::lock_guard<M> lock(mtx);
		++count;
		cond.notify_one();
	}

	/**
	 * @brief Hold the thread until notified
	 * Locks the thread until notified. Handles sporadic wake ups.
	 */
	inline void wait()
	{
		std::lock_guard<M> lock(mtx);
		while (!count)
			cond.wait(lock);
		--count;
	}

	/**
	 * @brief Check if a wait should be done
	 * @return True for waiting, false otherwise
	 * Checks if the resource counter is full. If not, it will reduce the
	 * counter once and tell it has done so.
	 */
	inline bool try_wait()
	{
		std::lock_guard<M> lock(mtx);
		if (count)
		{
			--count;
			return true;
		}
		return false;
	}

private:
	M mtx;
	C cond;
	mutable std::size_t count = 0;
};

/**
 * Base implementation with std library
 */
using semaphore = basic_semaphore<std::mutex, std::condition_variable>;

#endif // SEMAPHORE_HPP
