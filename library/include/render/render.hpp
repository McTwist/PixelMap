#pragma once
#ifndef RENDER_HPP
#define RENDER_HPP

#include "render/renderpassbuilder.hpp"
#include "blockcolor.hpp"
#include "eventhandler.hpp"

#include <string>
#include <functional>
#include <memory>

class ChunkRender;
class RegionRender;
class WorldRender;
struct ChunkRenderData;
struct RegionRenderData;
struct ImageRenderData;

/**
 * @brief Namespace for render structures
 */
namespace Render
{
	/**
	 * @brief Set the mode to draw
	 * Chunk - One image per chunk.
	 * Region - One image per region.
	 * Image - One image per world.
	 * Image Direct - One image per world. Optimized by drawing directly to image.
	 * Chunk Tiny - Shrinks each chunk to one pixel and puts into one image.
	 * Region Tiny - Shrinks each region to one pixel and puts into one image.
	 */
	enum class Mode
	{
		CHUNK,
		REGION,
		IMAGE,
		IMAGE_DIRECT, // Unusued, outdated, TODO: investigate
		CHUNK_TINY, // Debug
		REGION_TINY, // Debug
		DEFAULT = IMAGE
	};

	/**
	 * @brief Merge two chunk renders
	 * This will blend each pixel, putting one on top of the other.
	 * @param to The data to merge to.
	 * @param from The data to merge from.
	 */
	void merge(std::shared_ptr<ChunkRenderData> & to, const std::shared_ptr<ChunkRenderData> & from);
}

/**
 * @brief Settings for a render
 */
struct RenderSettings
{
	Render::Mode mode;
	std::string path;
	BlockColor colors;
	EventHandler<void(int)> events;
};

/**
 * @brief Rendering a chunk
 */
class ChunkRender
{
public:
	ChunkRender(std::shared_ptr<RenderSettings> setting);

	/**
	 * @brief Draw this specific chunk
	 * @param chunk The chunk to draw
	 * @param func The function to use when rendering
	 * @return The rendering of the chunk
	 */
	std::shared_ptr<ChunkRenderData> draw(const Chunk & chunk, RenderPassFunction func);

private:
	std::shared_ptr<RenderSettings> setting;

	/**
	 * @brief Generates palette from chunk
	 * @param chunk The chunk to fetch palette
	 * @param data The data to modify the palette
	 * @return If the palette is valid
	 */
	bool generatePalette(const Chunk & chunk, std::shared_ptr<ChunkRenderData> & data);
};

/**
 * @brief Rendering a region
 */
class RegionRender
{
public:
	RegionRender(std::shared_ptr<RenderSettings> setting);

	/**
	 * @brief Add a chunk to be rendered on that specific position in the region
	 * @param x Position
	 * @param z Position
	 * @return A renderer for the chunk
	 */
	void add(const std::shared_ptr<ChunkRenderData> & data);

	/**
	 * @brief Draw a region at this position
	 * @param x Position
	 * @param z Position
	 */
	std::shared_ptr<RegionRenderData> draw(int x, int z);

private:
	std::vector<std::shared_ptr<struct ChunkRenderData>> chunks;
	std::shared_ptr<RenderSettings> setting;
};

/**
 * @brief Render a whole world
 */
class WorldRender
{
public:
	/**
	 * @brief Constructor
	 * @param setting Settings for the render
	 * @param colors Colors to be used for palette
	 */
	WorldRender(std::shared_ptr<RenderSettings> setting);

	/**
	 * @brief Add a region to be rendered on that specific position in the world
	 * @param x Position
	 * @param z Position
	 * @return A renderer for the region
	 */
	void add(std::shared_ptr<RegionRenderData> & data);

	/**
	 * @brief Draw all regions and chunks
	 */
	void draw();

	/**
	 * @brief Set an event callback for each region rendered
	 * @param func A callback which each event tells a region was rendered along with the maount of chunks
	 */
	void eventRenderRegion(std::function<void(int)> && func);

private:
	// Note: pockets<std::list, int, pockets<std::vector, int, RedionRenderData>> could be an alternative structure representation
	std::unordered_map<utility::RegionPosition, std::shared_ptr<RegionRenderData>> regions;
	std::shared_ptr<RenderSettings> setting;

	void calculateBoundary(std::shared_ptr<ImageRenderData> & data);
};

#endif // RENDER_HPP
