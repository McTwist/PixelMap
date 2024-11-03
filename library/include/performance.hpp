#pragma once
#ifndef PERFORMANCE_HPP

#include "timer.hpp"

#include <functional>

/**
 * @brief Check performance how long a task will take
 * @tparam Precision The precision of the returned value
 * @tparam Ratio The ratio of the elapsed time
 * @param func The function to process
 * @param count The amount of times it will run the task, to mean the total process time
 * @return The time it took to process the task, in ratio
 */
template<typename Precision = double, class Ratio = std::ratio<1>>
static Precision checkPerformance(std::function<void()> func, std::size_t count = 1)
{
	Timer<Precision> timer;
	Precision total = 0;
	for (std::size_t i = 0; i < count; ++i)
	{
		timer.start();
		func();
		total += timer.template elapsed<Ratio>();
	}
	return total / count;
}

/**
 * @brief Check the performance for the specific part of code
 * @param FUNC The function, in curly brackets, that will be run
 * @param ADD The function that the time will be added into
 */
#ifdef PERF_DEBUG
#define PERFORMANCE(FUNC, ADD) do { auto a = checkPerformance<double, std::milli>([&]() FUNC); ADD(a); } while(false)
#else
#define PERFORMANCE(FUNC, ADD) do FUNC while(false)
#endif

#endif // PERFORMANCE_HPP
