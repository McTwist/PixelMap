#include "profiler.hpp"

#include <new>
#include <cstdlib>
#include <iostream>
#include <atomic>

// Note: There is a possibility that these will run out (*wheeze*)
static std::atomic_uint64_t s_number_of_allocs = 0;
static std::atomic_uint64_t s_number_of_deallocs = 0;
static std::atomic_uint64_t s_sum_of_allocs = 0;
static std::atomic_uint64_t s_sum_of_deallocs = 0;
static std::atomic_uint64_t s_sum_max_allocs = 0;
static std::atomic_uint64_t s_max_alloc_size = 0;

static inline void update_max(std::size_t size);

static inline void update_max(std::size_t size)
{
	for (auto sum = s_sum_of_allocs - s_sum_of_deallocs; sum > s_sum_max_allocs; sum = s_sum_of_allocs - s_sum_of_deallocs)
		s_sum_max_allocs = sum;
	if (size > s_max_alloc_size)
		s_max_alloc_size = size;
}

void* operator new(std::size_t size)
{
	++s_number_of_allocs;
	s_sum_of_allocs += size;
	update_max(size);
	void *p = malloc(size);
	if (!p) throw std::bad_alloc();
	return p;
}

void* operator new[](std::size_t size)
{
	++s_number_of_allocs;
	s_sum_of_allocs += size;
	update_max(size);
	void *p = malloc(size);
	if (!p) throw std::bad_alloc();
	return p;
}

void* operator new[](std::size_t size, const std::nothrow_t&) throw()
{
	++s_number_of_allocs;
	s_sum_of_allocs += size;
	update_max(size);
	return malloc(size);
}
void* operator new(std::size_t size, const std::nothrow_t&) throw()
{
	++s_number_of_allocs;
	s_sum_of_allocs += size;
	update_max(size);
	return malloc(size);
}


void operator delete(void* ptr) throw()
{
	++s_number_of_deallocs;
	free(ptr);
}
void operator delete(void* ptr, std::size_t size) throw()
{
	++s_number_of_deallocs;
	s_sum_of_deallocs += size;
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
void operator delete[](void* ptr, std::size_t size) throw()
{
	++s_number_of_deallocs;
	s_sum_of_deallocs += size;
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
	sum_of_deallocs = s_sum_of_deallocs;
	sum_min_allocs = s_sum_max_allocs;
}

void MemoryProfiler::print() const
{
	auto allocs = int64_t(s_number_of_allocs - number_of_allocs);
	auto deallocs = int64_t(s_number_of_deallocs - number_of_deallocs);
	auto sum_allocs = int64_t(s_sum_of_allocs - sum_of_allocs);
	auto sum_deallocs = int64_t(s_sum_of_deallocs - sum_of_deallocs);
	auto sum_max = int64_t(s_sum_max_allocs - sum_min_allocs);
	auto diff_allocs = allocs - deallocs;
	auto diff_sum = sum_allocs - sum_deallocs;
	std::cout << "Allocs, deallocs, diff: " << allocs << " ; " << deallocs << " ; " << diff_allocs << std::endl;
	std::cout << "Sum allocations: " << sum_allocs << std::endl;
	std::cout << "Sum deallocations: " << sum_deallocs << std::endl;
	std::cout << "Diff allocations: " << diff_sum << std::endl;
	std::cout << "Avg. allocation size, max allocation size: " << (sum_allocs / allocs) << " ; " << s_max_alloc_size << std::endl;
	std::cout << "Max allocation: " << sum_max << std::endl;
}

