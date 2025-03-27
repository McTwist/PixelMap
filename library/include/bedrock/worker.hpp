#pragma once
#ifndef BEDROCK_WORKER_HPP
#define BEDROCK_WORKER_HPP

#include "../worker.hpp"

#include "lightsource.hpp"

namespace LevelDB
{
class LevelFile;
}
namespace bedrock
{
class World;
}
struct RegionRenderData;

namespace bedrock
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
	LightSource light_source;
	bool night_mode;

	/**
	 * @brief Working on a level file
	 * @param file Data from a level file
	 * @param dimension The dimension to read from the file
	 * @return A world from the file, chunks being prerendered
	 */
	std::shared_ptr<bedrock::World> workFile(std::shared_ptr<LevelDB::LevelFile> file, int32_t dimension);

	/**
	 * @brief Merge two worlds
	 * @param f1 First world
	 * @param f2 Second world
	 * @return The merged world
	 */
	std::shared_ptr<bedrock::World> mergeWorlds(std::shared_future<std::shared_ptr<bedrock::World>> f1, std::shared_future<std::shared_ptr<World>> f2);

	/**
	 * @brief Render a region with chunk renders
	 * @param pos The position of the render
	 * @param chunks The chunks to render
	 * @return A rendered region data
	 */
	std::shared_ptr<RegionRenderData> renderRegion(utility::RegionPosition pos, std::vector<std::shared_ptr<ChunkRenderData>> chunks);
};

}

#endif // BEDROCK_WORKER_HPP
