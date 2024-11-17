#include "bedrock/world.hpp"

#include "bedrock/limits.hpp"
#include "render/render.hpp"
#include "minecraft.hpp"
#include "performance.hpp"

#include <spdlog/spdlog.h>

#include <array>
#include <queue>

namespace bedrock
{

World::World(const std::string & _name, int32_t _dimension)
	: name(_name), dimension(_dimension)
{
}

Chunk & World::getChunk(int32_t x, int32_t y)
{
	auto pos = utility::PlanePosition(x, y);
	return chunks[pos];
}

void World::setSection(int32_t x, int32_t y, const SectionData & section)
{
	auto pos = utility::PlanePosition(x, y);
	chunks.emplace(pos, Chunk()).first->second.setSection(section);
}

void World::setHeightmap(int32_t x, int32_t y, const std::vector<int32_t> & heightMap)
{
	auto pos = utility::PlanePosition(x, y);
	chunks.emplace(pos, Chunk()).first->second.setHeightMap(heightMap);
}

void World::generateBlockLight(const LightSource & lightsource)
{
	constexpr auto CHUNK_WIDTH = Minecraft::chunkWidth();
	constexpr auto SECTION_SIZE = CHUNK_WIDTH * CHUNK_WIDTH * CHUNK_WIDTH;
	#define INDEX(x, y, z) (mod(x, CHUNK_WIDTH) * CHUNK_WIDTH * CHUNK_WIDTH + mod(y, CHUNK_WIDTH) + mod(z, CHUNK_WIDTH) * CHUNK_WIDTH)

	using namespace utility::math;
	using namespace utility::coord;
	if (chunks.empty())
		return;
	// Find world min-max
	auto yMax = std::numeric_limits<int32_t>::min(), yMin = std::numeric_limits<int32_t>::max(),
		xMin = std::numeric_limits<int32_t>::max(), zMin = std::numeric_limits<int32_t>::max();
	auto perf_minmax = checkPerformance<>([&]() {
		for (auto & [pos, chunk] : chunks)
		{
			if (chunk.getMaxY() > yMax)
				yMax = chunk.getMaxY();
			if (chunk.getMinY() < yMin)
				yMin = chunk.getMinY();
			if (pos.x < xMin)
				xMin = pos.x;
			if (pos.y < zMin)
				zMin = pos.y;
		}
	});
	// Locate lights
	std::array<std::vector<utility::BlockPosition>, 15> queues;
	std::unordered_map<glm::ivec3, std::array<uint8_t, SECTION_SIZE>> blocklight;
	int light_palettes = 0;
	std::unordered_map<utility::ChunkPosition, int> airs;
	auto perf_locate = checkPerformance<>([&]() {
		for (auto & [pos, chunk] : chunks)
		{
			auto & palette = chunk.getNSPalette();
			std::unordered_map<uint16_t, uint8_t> light_palette;
			for (std::size_t n = 0; n < palette.size(); ++n)
			{
				if (lightsource.isLightSource(palette[n]))
					light_palette[n] = lightsource.getLightPower(palette[n]);
				if (palette[n] == "minecraft:air")
					airs[pos] = n;
			}
			light_palettes += light_palette.size();
			for (uint32_t x = 0; x < CHUNK_WIDTH; ++x)
			{
				for (uint32_t z = 0; z < CHUNK_WIDTH; ++z)
				{
					for (int y = yMax; y >= yMin; --y)
					{
						utility::BlockPosition blockPos{x, y, z};
						if (!chunk.hasSection(blockPos))
						{
							y -= SECTION_Y - 1;
							continue;
						}
						auto lit = light_palette.find(chunk.getTile(blockPos).index);
						if (lit == light_palette.end())
							continue;
						auto sectionY = toSection(y);
						auto light = lit->second;
						auto lightPos = utility::BlockPosition{pos.x * int(CHUNK_WIDTH) + int(x), y, pos.y * int(CHUNK_WIDTH) + int(z)};
						queues[15 - light].emplace_back(lightPos);
						blocklight[{pos.x, sectionY, pos.y}][INDEX(x, y, z)] = light;
					}
				}
			}
		}
	});
	// Flood
	std::array<utility::BlockPosition, 6> directions{};
	directions[0].x = 1;
	directions[1].x = -1;
	directions[2].y = 1;
	directions[3].y = -1;
	directions[4].z = 1;
	directions[5].z = -1;
	constexpr auto last = 14;
	auto perf_flood = checkPerformance<>([&]() {
		for (int i = 0; i < last; ++i)
		{
			int8_t light = 15 - i - 1;
			auto & queue = queues[i];
			auto & next_queue = queues[i+1];
			next_queue.reserve(queue.size() * 3 + next_queue.size());
			for (auto pos : queue)
			{
				for (auto & dir : directions)
				{
					auto new_pos = pos + dir;
					auto sec_pos = toSection(new_pos);
					auto pl = utility::ChunkPosition{sec_pos.x, sec_pos.z};
					auto it = chunks.find(pl);
					if (it == chunks.end())
						continue;
					if (!chunks[pl].hasSection(new_pos))
						continue;
					auto n = INDEX(mod(new_pos.x, CHUNK_WIDTH), mod(new_pos.y, CHUNK_WIDTH), mod(new_pos.z, CHUNK_WIDTH));
					if (blocklight[sec_pos][n] < light)
					{
						blocklight[sec_pos][n] = light;
						auto air = airs.find(pl) != airs.end() ? airs[pl] : -1;
						if (i < last-1 && chunks[pl].getTile(new_pos).index == air)
							next_queue.emplace_back(new_pos);
					}
				}
			}
			queue.clear();
		}
	});
	// Apply
	auto perf_apply = checkPerformance<>([&]() {
		for (auto & [pos, light] : blocklight)
		{
			auto sec_pos = pos;
			sec_pos.y *= CHUNK_WIDTH;
			auto pl = utility::ChunkPosition{pos.x, pos.z};
			auto section = chunks[pl].getSection(sec_pos);
			section.updateBlockLight(light);
			chunks[pl].updateSection(section);
		}
	});
	spdlog::debug("MinMax: {:f}", perf_minmax);
	spdlog::debug("Locate: {:f}", perf_locate);
	spdlog::debug("BFS: {:f}", perf_flood);
	spdlog::debug("Apply: {:f}", perf_apply);
}

void World::filter(const std::function<bool(const Chunk &)> & f)
{
	// TODO: Replace with std::erase_if (C++20)
	for (auto first = chunks.begin(), last = chunks.end(); first != last;)
	{
		if (f(first->second))
			first = chunks.erase(first);
		else
			++first;
	}
}

void World::draw(const std::function<std::shared_ptr<ChunkRenderData>(const Chunk &)> & render)
{
	renderData.reserve(chunks.size());
	for (auto & it : chunks)
		renderData.emplace(it.first, render(it.second));

	chunks.clear();
}

void World::merge(const World & world)
{
	for (auto & it : world.chunks)
	{
		auto at = chunks.find(it.first);
		if (at == chunks.end())
			chunks.emplace(it.first, it.second);
		else
			at->second.merge(it.second);
	}

	for (auto & it : world.renderData)
	{
		auto at = renderData.find(it.first);
		if (at != renderData.end())
			//spdlog::info("Replacing: {:s} -> {:s} ({:d}, {:d})", world.name, name, it.first.x, it.first.y);
			Render::merge(at->second, it.second);
		else
			renderData.emplace(it.first, it.second);
	}
}

}
