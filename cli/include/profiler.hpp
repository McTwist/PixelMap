#pragma once
#ifndef PROFILER_HPP
#define PROFILER_HPP

#include <cstdint>

class MemoryProfiler
{
public:
	MemoryProfiler();
	void reset();
	void print() const;
private:
	uint64_t number_of_allocs = 0;
	uint64_t number_of_deallocs = 0;
	uint64_t sum_of_allocs = 0;
};

#endif // PROFILER_HPP
