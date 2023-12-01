#pragma once
#ifndef PIXELMAP_HPP

#include "options.hpp"
#include "threadworker.hpp"
#include "eventhandler.hpp"

#include <string>
#include <functional>
#include <atomic>

/**
 * @brief PixelMap library
 * PixelMap is a library that convert minecraft dimensions to rendered images
 * which is either one large or several smaller.
 * Several options can be sent to change the input, output and rendering.
 */
class PixelMap final
{
public:
	/**
	 * @brief PixelMap constructor and destructor
	 */
	PixelMap();
	~PixelMap();

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
	 * @brief Set an event function when the processing is done
	 * @param func The function to call whenver processing is done
	 */
	void eventDone(std::function<void()> && func);

	/**
	 * @brief Set options to change the internal functionality
	 * @param options The options to set
	 */
	void set(const Options & options);

	/**
	 * @brief Start a task from a certain path
	 * @param path The path to process
	 * @param output The output path for finished work
	 */
	void start(const std::string & path, const std::string & output, int32_t dimension=0);

	/**
	 * @brief Stop all current tasks, aborting the whole operation
	 */
	void stop();

	/**
	 * @brief Wait until tasks are finished
	 */
	void wait();

	/**
	 * @brief Check if there is no tasks left
	 * @return False when work is processed, true otherwise
	 */
	bool done();
private:
	ThreadWorker worker;
	std::atomic_bool run;

	EventHandler<void(int)> func_totalChunks;
	EventHandler<void(int)> func_finishedChunk;
	EventHandler<void(int)> func_totalRender;
	EventHandler<void(int)> func_finishedRender;
	EventHandler<void()> func_done;

	Options options;

	/**
	 * @brief Do the work specified with the path
	 * @param path The path to process
	 * @param output The output path for finished work
	 */
	void work(const std::string & path, const std::string & output, int32_t dimension);
};

#endif // PIXELMAP_HPP
