#pragma once
#ifndef WORKER_BASE_HPP
#define WORKER_BASE_HPP

#include "options.hpp"
#include "render/render.hpp"
#include "render/utility.hpp"
#include "threadpool.hpp"
#include "eventhandler.hpp"
#include "shared_value.hpp"
#include "shared_counter.hpp"
#include "lonely.hpp"

#include <atomic>
#include <mutex>
#include <string>
#include <vector>

/**
 * @brief Keeps track of how many errors for specific tasks
 */
struct ErrorStats
{
	enum Type
	{
		ERROR_COMPRESSION,
		ERROR_TYPE,
		ERROR_PARSE,
		ERROR_EMPTY_CHUNKS,
		ERROR_EMPTY_REGIONS,
		ERROR_LONELY_CHUNKS,
		ERROR_COUNT
	};

	int errors[ERROR_COUNT] = { 0 };
	std::mutex _error_m[ERROR_COUNT];

	inline void report(Type type)
	{
		std::lock_guard<std::mutex> guard(_error_m[type]);
		++errors[type];
	}

	void print() const;
};

/**
 * @brief Keeps track of stats for performance
 */
class PerfStats
{
	struct PerfValue
	{
		PerfValue() = default;
		PerfValue(const std::string & _name) : name(_name) {}
		inline void update(double a) { value.update([&a](auto & v) { v += a; }); }
		std::string name;
		shared_value<double> value{0};
	};
public:

	std::size_t createPerfValue(const std::string & name);
	inline void addPerfValue(std::size_t id, double a)
	{
		assert(id < perfValues.size());
		perfValues[id]->update(a);
	}
	inline std::function<void(double)> getPerfValue(std::size_t id)
	{
		assert(id < perfValues.size());
		return [&value = perfValues[id]](auto a) { value->update(a); };
	}
	inline void addErrorString(const std::string & str) { errorString.update([&](auto & v) { v.push_back(str); }); }

	inline void regionCounterIncrease() { regionCounter.increase(); }
	inline void regionCounterDecrease() { regionCounter.decrease(); }
	inline void regionCounterAbort() { regionCounter.abort(); }

	ErrorStats errors;

	void print() const;

private:
	std::vector<std::shared_ptr<PerfValue>> perfValues;
	shared_value<std::vector<std::string>> errorString;
	shared_counter<int> regionCounter;
};

/**
 * @brief Outsourced worker to handle the whole work process
 */
class WorkerBase
{
public:
	/**
	 * @brief Constructor
	 * @param run Boolean if the worker is running
	 * @param options Options to be used to change how the worker behaves
	 */
	WorkerBase(std::atomic_bool & run, const Options & options);

	/**
	 * @brief Set an event function when total chunks is updated
	 * @param func The function to call whenever total chunks are updated
	 */
	void eventTotalChunks(std::function<void(int)> && func);

	/**
	 * @brief Set an event function when finished chunks are updated
	 * @param func The function to call whenever finished chunks are updated
	 */
	void eventFinishedChunk(std::function<void(int)> && func);

	/**
	 * @brief Set an event function when finished chunks are updated
	 * @param func The function to call whenever finished chunks are updated
	 */
	void eventFinishedChunk(std::function<void()> && func);

	/**
	 * @brief Set an event function when total rendering are updated
	 * @param func The function to call whenever total rendering are updated
	 */
	void eventTotalRender(std::function<void(int)> && func);

	/**
	 * @brief Set an event function when finished rendering are updated
	 * @param func The function to call whenever finished rendering are updated
	 */
	void eventFinishedRender(std::function<void(int)> && func);

	/**
	 * @brief Set an event function when finished rendering are updated
	 * @param func The function to call whenever finished rendering are updated
	 */
	void eventFinishedRender(std::function<void()> && func);

	/**
	 * @brief Do the work specified with the path
	 * @param path The path to process
	 * @param output The output path for finished work
	 * @param dimension The dimension of the world
	 */
	virtual void work(const std::string & path, const std::string & output, int32_t dimension) = 0;

protected:
	std::atomic_bool & run;
	ThreadPool pool;
	std::shared_ptr<RenderSettings> settings;
	RenderPassFunction renderPass;

	std::atomic_ulong total_chunks;
	std::atomic_ulong total_regions;
	Lonely lonely;


	EventHandler<void(int)> func_totalChunks;
	EventHandler<void(int)> func_finishedChunk;
	EventHandler<void()> func_finishedChunks;
	EventHandler<void(int)> func_totalRender;
	EventHandler<void(int)> func_finishedRender;
	EventHandler<void()> func_finishedRenders;
	EventHandler<void()> func_done;

	PerfStats perf;
};

#endif // WORKER_BASE_HPP
