#pragma once
#ifndef BETA_WORKER_HPP
#define BETA_WORKER_HPP

#include "../worker.hpp"

namespace region
{
class RegionFile;
class ChunkData;
}
struct RegionRenderData;
struct ChunkRenderData;

namespace beta
{

/**
 * @brief Outsourced worker to handle the whole work process
 */
class Worker : public WorkerBase
{
public:
	Worker(std::atomic_bool & run, const Options & options);

	/**
	 * @brief Do the work specified with the path
	 * @param path The path to process
	 * @param output The output path for finished work
	 * @param dimension The dimension of the world
	 */
	void work(const std::string & path, const std::string & output, int32_t dimension) override;

private:

	/**
	 * @brief The region to work on
	 * @param Data from region used for the region
	 * @param A renderer for the region
	 */
	std::future<std::shared_ptr<RegionRenderData>> workRegion(std::shared_ptr<region::RegionFile>, int i);

	/**
	 * @brief The chunk to work on
	 * @param Data from the chunk used
	 * @param A renderer for the chunk
	 * @param Specialized region data
	 */
	std::shared_ptr<ChunkRenderData> workChunk(std::shared_ptr<region::ChunkData>);
};

} // namespace beta

#endif // BETA_WORKER_HPP
