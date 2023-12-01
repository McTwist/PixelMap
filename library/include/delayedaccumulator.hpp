#pragma once
#ifndef DELAYED_EVENT_HPP
#define DELAYED_EVENT_HPP

#include "timer.hpp"

#include <mutex>
#include <functional>

/**
 * @brief Delay an input to be flushed later
 * Allows lots of values to be added, but it will not flush the value to the
 * callback unless it was called within the delay or specifically flushed.
 */
class DelayedAccumulator
{
	typedef std::function<void(int)> Event;
public:
	/**
	 * @brief Constructor
	 * Default is disabled, as an event is required
	 */
	DelayedAccumulator() = delete;
	DelayedAccumulator(Event _event, float _delay = 1.0f)
		: delay(_delay), event(_event)
	{
		timer.start();
	}

	/**
	 * @brief Destructor
	 */
	~DelayedAccumulator()
	{
		if (got_data)
			flush();
	}

	/**
	 * @brief Set a new event
	 * @param func The callback
	 */
	void setEvent(Event func)
	{
		std::lock_guard<std::mutex> guard(mutex);
		event = func;
	}

	/**
	 * @brief Set a new delay
	 * @param _delay The delay to update
	 */
	void setDelay(float _delay)
	{
		if (_delay < 0)
			return;
		std::lock_guard<std::mutex> guard(mutex);
		delay = _delay;
	}

	/**
	 * @brief Add a value to be flushed
	 * @param v The value to add
	 */
	void add(int v)
	{
		std::lock_guard<std::mutex> guard(mutex);
		sum += v;
		if (timer.elapsed() >= delay)
		{
			send();
			got_data = false;
		}
		else
		{
			got_data = true;
		}
	}

	/**
	 * @brief Flush the value to the callback
	 */
	void flush()
	{
		std::lock_guard<std::mutex> guard(mutex);
		if (got_data)
		{
			send();
			got_data = false;
		}
	}

private:

	/**
	 * @brief Send the value to the callback
	 * Note: Only to be called when blocked
	 */
	void send()
	{
		event(sum);
		sum = 0;
		timer.start();
	}

	bool got_data = false;
	int sum = 0;
	float delay;
	Event event = [](int){};
	Timer<> timer;
	std::mutex mutex;
};

#endif // DELAYED_EVENT_HPP
