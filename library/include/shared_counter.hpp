#pragma once
#ifndef SHARED_COUNTER_HPP
#define SHARED_COUNTER_HPP

#include <mutex>
#include <condition_variable>

// TODO: Replace with atomic? (C++11)

/**
 * @brief Shares a counter between threads
 * When reaching target value, a notification will be sent to waiting threads
 */
template<typename T>
class shared_counter
{
public:
	/**
	 * @brief Constructor
	 * @param t Target value
	 * @param v Counter initial value
	 */
	shared_counter() = default;
	shared_counter(const T & t)
		: counter(T()), target(t)
	{}
	shared_counter(const T & t, const T & v)
		: counter(v), target(t)
	{}

	/**
	 * @brief Set counter
	 * @param v The value to set
	 */
	void set(const T & v)
	{
		std::lock_guard<std::mutex> lock(mutex);
		counter = v;
	}

	/**
	 * @brief Decrease the counter
	 */
	void decrease()
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (--counter == target)
			cond.notify_all();
	}

	/**
	 * @brief Increase the counter
	 */
	void increase()
	{
		std::lock_guard<std::mutex> lock(mutex);
		if (++counter == target)
			cond.notify_all();
	}

	/**
	 * @brief Abort counter by setting counter to target value
	 */
	void abort()
	{
		std::lock_guard<std::mutex> lock(mutex);
		counter = target;
		cond.notify_all();
	}

	/**
	 * @brief Wait until target is reached
	 */
	void wait() const
	{
		std::unique_lock<std::mutex> lock(mutex);
		cond.wait(lock, [this]{ return counter == target; });
		lock.unlock();
	}

	/**
	 * @brief Set and get counter quickly
	 * @param v The value to set counter
	 * @return The value set
	 */
	T operator=(const T & v)
	{
		set(v);
		return v;
	}

private:
	T counter = T();
	const T target = T();
	mutable std::mutex mutex;
	mutable std::condition_variable cond;
};

#endif // SHARED_COUNTER_HPP
