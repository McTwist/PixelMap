#pragma once
#ifndef RENDERPASSDEFINE_HPP
#define RENDERPASSDEFINE_HPP

#include "render/utility.hpp"
#include "minecraft.hpp"

#include <vector>

// TODO: Static, make more dynamic for rotation support
constexpr int32_t CHUNK_WIDTH = int32_t(Minecraft::chunkWidth());
constexpr int32_t REGION_COUNT = int32_t(Minecraft::regionWidth());
constexpr int32_t REGION_WIDTH = CHUNK_WIDTH * REGION_COUNT;
constexpr int32_t CHUNK_COUNT = REGION_COUNT * REGION_COUNT;

struct AABB
{
	int ax = 0, az = 0;
	int bx = CHUNK_WIDTH, bz = CHUNK_WIDTH;
};

struct ChunkRenderData
{
	int32_t x = 0, z = 0;
	std::vector<utility::RGBA> palette;
	std::vector<utility::RGBA> scratch;
};

struct RegionRenderData
{
	int32_t x = 0, z = 0;
	std::vector<utility::RGBA> scratchRegion;
	std::vector<std::vector<utility::RGBA>> scratchImage;
};

struct ImageRenderData
{
	AABB boundary;
};

namespace RenderPass
{

std::vector<utility::RGBA> shrinkRegion(const std::vector<utility::RGBA> & from);

}


#endif //RENDERPASSDEFINE_HPP
