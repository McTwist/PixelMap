#pragma once
#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP

#include <vector>
#include <functional>
#include <mutex>

/**
 * @brief Store a list of callbacks to be called
 */
template<class F>
class EventHandler
{
public:
	/**
	 * @brief Add a callback to be called later
	 * @param func The callback to store away
	 */
	void add(std::function<F> && func)
	{
		std::lock_guard<std::mutex> guard(mutex);
		functions.emplace_back(func);
	}

	/**
	 * @brief Call the callbacks
	 * @param args Arguments for the callbacks
	 */
	template<typename... Args>
	void call(Args... args)
	{
		std::lock_guard<std::mutex> guard(mutex);
		for (auto func : functions)
		{
			func(args...);
		}
	}

private:
	std::vector<std::function<F>> functions;
	std::mutex mutex;
};

#endif // EVENTHANDLER_HPP
