#include "profiler.hpp"

#include <new>
#include <cstdlib>
#include <iostream>

// Note: There is a possibility that these will run out (*wheeze*)
static uint64_t s_number_of_allocs = 0;
static uint64_t s_number_of_deallocs = 0;
static uint64_t s_sum_of_allocs = 0;

void* operator new(std::size_t size)
{
	++s_number_of_allocs;
	s_sum_of_allocs += size;
	void *p = malloc(size);
	if (!p) throw std::bad_alloc();
	return p;
}

void* operator new[](std::size_t size)
{
	++s_number_of_allocs;
	s_sum_of_allocs += size;
	void *p = malloc(size);
	if (!p) throw std::bad_alloc();
	return p;
}

void* operator new[](std::size_t size, const std::nothrow_t&) throw()
{
	++s_number_of_allocs;
	s_sum_of_allocs += size;
	return malloc(size);
}
void* operator new(std::size_t size, const std::nothrow_t&) throw()
{
	++s_number_of_allocs;
	s_sum_of_allocs += size;
	return malloc(size);
}


void operator delete(void* ptr) throw()
{
	++s_number_of_deallocs;
	free(ptr);
}
void operator delete(void* ptr, const std::nothrow_t&) throw()
{
	++s_number_of_deallocs;
	free(ptr);
}
void operator delete[](void* ptr) throw()
{
	++s_number_of_deallocs;
	free(ptr);
}
void operator delete[](void* ptr, const std::nothrow_t&) throw()
{
	++s_number_of_deallocs;
	free(ptr);
}

MemoryProfiler::MemoryProfiler()
{
	reset();
}

void MemoryProfiler::reset()
{
	// Set the starting number
	number_of_allocs = s_number_of_allocs;
	number_of_deallocs = s_number_of_deallocs;
	sum_of_allocs = s_sum_of_allocs;
}

void MemoryProfiler::print() const
{
	auto allocs = int64_t(s_number_of_allocs - number_of_allocs);
	auto deallocs = int64_t(s_number_of_deallocs - number_of_deallocs);
	auto sum = int64_t(s_sum_of_allocs - sum_of_allocs);
	auto diff = allocs - deallocs;
	std::cout << "Allocs, deallocs, diff: " << allocs << " , " << deallocs << " , " << diff << std::endl;
	std::cout << "Sum allocations: " << sum << std::endl;
	std::cout << "Avg. allocation size: " << (sum / allocs) << std::endl;
}

