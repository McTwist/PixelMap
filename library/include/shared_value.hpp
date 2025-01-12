#pragma once
#ifndef SHARED_VALUE_HPP
#define SHARED_VALUE_HPP

#include <mutex>
#include <shared_mutex>

/**
 * @brief Shares a value between threads, keeping it safe
 */
template<typename T>
class shared_value
{
	using shared_mutex = std::shared_timed_mutex;
public:
	/**
	 * @brief Constructor
	 */
	shared_value()
	{}
	shared_value(const T & v)
		: value(v)
	{}

	/**
	 * @brief Get value
	 * @return The internal value
	 */
	inline T get() const
	{
		std::shared_lock<shared_mutex> lock(mutex);
		return value;
	}

	/**
	 * @brief Set a value
	 * @param v The value to set
	 */
	inline void set(const T & v)
	{
		std::lock_guard<shared_mutex> lock(mutex);
		value = v;
	}

	/**
	 * @brief Update the value
	 * @param f Function which is called when the value can safely be edited
	 */
	inline void update(std::function<void(T &)> f)
	{
		std::lock_guard<shared_mutex> lock(mutex);
		f(value);
	}

	/**
	 * @brief Fetch the value with a function
	 * @param f The function that will get the value
	 * @return The value retrieved
	 */
	template<typename R>
	inline R fetch(std::function<R(const T &)> f)
	{
		std::shared_lock<shared_mutex> lock(mutex);
		return f(value);
	}

	/**
	 * @brief Update and fetch the value
	 * @param f A function to edit and retrieve the value
	 * @return The value retrieved
	 */
	template<typename R>
	inline R update_fetch(std::function<R(T &)> f)
	{
		std::lock_guard<shared_mutex> lock(mutex);
		return f(value);
	}

	/**
	 * @brief Implicit get the value
	 */
	inline operator T() const
	{
		return get();
	}

	/**
	 * @brief Implicit set and get the value
	 * @param v The value given
	 * @return The value retrieved
	 */
	inline T operator=(const T & v)
	{
		set(v);
		return v;
	}

private:
	T value = T();
	mutable shared_mutex mutex;
};

#endif // SHARED_VALUE_HPP
