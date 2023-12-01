#pragma once
#ifndef TIMER_HPP
#define TIMER_HPP

#include <chrono>

/**
 * @brief Timer
 * @tparam T The type that will be returned
 * Keeps track of a timer to determine how long time have elapsed.
 * It is only intended to wrap around certain structures to make it easier
 * to handle.
 */
template<typename T = float>
class Timer
{
	typedef std::chrono::high_resolution_clock Clock;
	typedef Clock::time_point TimePoint;
public:

	/**
	 * @brief Starts the timer
	 */
	inline void start()
	{
		_start = Clock::now();
	}

	/**
	 * @brief Returns the elapsed time
	 * @tparam R The ratio of the returned value, defaults to seconds
	 * @return The time since start
	 * For the R, std includes several typedefs for common SI ratios.
	 */
	template<class R = std::ratio<1>>
	inline T elapsed() const
	{
		using namespace std::chrono;
		return duration_cast<duration<T, R>>(Clock::now() - _start).count();
	}

	/**
	 * @brief Ends the time, returning the elapsed time, and starts it again
	 * @tparam R The ratio of the returned value, defaults to seconds
	 * @return The time since start
	 * For the R, std includes several typedefs for common SI ratios.
	 */
	template<class R = std::ratio<1>>
	inline T restart()
	{
		T time = elapsed<R>();
		start();
		return time;
	}

private:
	TimePoint _start;
};

#endif // TIMER_HPP
