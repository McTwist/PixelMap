#include "render/render.hpp"

#include "render/image.hpp"
#include "render/utility.hpp"
#include "platform.hpp"
#include "string.hpp"
#include "minecraft.hpp"
#include "chunk.hpp"

#include <algorithm>

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

template<typename T>
void fillPalette(const std::vector<T> & src, const BlockColor & colors, std::vector<utility::RGBA> & dst)
{
	dst.resize(src.size());
	for (auto i = 0U; i < src.size(); ++i)
		dst[i] = colors.getColor(colors.getIndex(src[i]));
}

struct ChunkRenderData
{
	int32_t x = 0, z = 0;
	std::vector<utility::RGBA> palette;
	std::vector<utility::RGBA> scratch;
};

void Render::merge(std::shared_ptr<ChunkRenderData> & to, const std::shared_ptr<ChunkRenderData> & from)
{
	if (to->scratch.empty())
		to->scratch = std::move(from->scratch);
	else if (to->scratch.size() == from->scratch.size())
		std::transform(
			to->scratch.begin(), to->scratch.end(),
			from->scratch.begin(), to->scratch.begin(),
			[](const utility::RGBA & a, const utility::RGBA & b) -> utility::RGBA {
				return utility::color::blend(b, a);
			});
}

ChunkRender::ChunkRender(std::shared_ptr<RenderSettings> _setting)
	: setting(_setting)
{
}

std::shared_ptr<ChunkRenderData> ChunkRender::draw(const Chunk & chunk, RenderPassFunction func)
{
	std::shared_ptr<ChunkRenderData> data = std::make_shared<ChunkRenderData>();

	if (!generatePalette(chunk, data))
		return data;

	data->x = chunk.getX();
	data->z = chunk.getZ();

	switch (setting->mode)
	{
	case Render::Mode::CHUNK:
		{
			auto & palette = data->palette;
			std::vector<utility::RGBA> row(CHUNK_WIDTH);
			// TODO: Test if this is reasonable
			auto cx = utility::math::mod(chunk.getX(), CHUNK_WIDTH);
			auto cz = utility::math::mod(chunk.getZ(), CHUNK_WIDTH);
			auto rx = chunk.getX() / CHUNK_WIDTH;
			auto rz = chunk.getZ() / CHUNK_WIDTH;
			auto output = platform::path::join(setting->path, string::format("r.", rx, ".", rz));
			platform::path::mkdir(output);
			auto path = platform::path::join(output, string::format("chunk.", cx, ".", cz, ".png"));
			Image image(CHUNK_WIDTH, CHUNK_WIDTH);
			image.save(path, [&row, &palette, &chunk, func](uint32_t bz)
			{
				std::fill(row.begin(), row.end(), utility::RGBA());
				for (uint32_t bx = 0; bx < CHUNK_WIDTH; ++bx)
				{
					using namespace utility;
					RenderPassData data{palette, chunk, Direction(0, -1, 0), Vector(bx, chunk.getMaxY(), bz), RGBA()};
					func(data);
					row[bx] = data.color;
				}
				return row;
			});
			setting->events.call(1);
		}
		break;
	case Render::Mode::REGION:
		platform::path::mkdir(setting->path);
		// Fall-through as it still needs to render each chunk
		[[fallthrough]];
	case Render::Mode::IMAGE:
	case Render::Mode::IMAGE_DIRECT:
		{
			auto & area	= data->scratch;
			auto & palette = data->palette;
			area.resize(CHUNK_WIDTH * CHUNK_WIDTH);
			for (int32_t bz = 0; bz < CHUNK_WIDTH; ++bz)
			{
				for (int32_t bx = 0; bx < CHUNK_WIDTH; ++bx)
				{
					using namespace utility;
					RenderPassData data{palette, chunk, Direction(0, -1, 0), Vector(bx, chunk.getMaxY(), bz), RGBA()};
					func(data);
					area[utility::math::index2d(CHUNK_WIDTH, bx, bz)] = data.color;
				}
			}
		}
		break;
	case Render::Mode::CHUNK_TINY:
		{
			auto & area	= data->scratch;
			area.resize(1);
			area[0] = utility::RGBA(255, 0, 0, 255);
		}
		break;
	case Render::Mode::REGION_TINY:
		break;
	}
	return data;
}

bool ChunkRender::generatePalette(const Chunk & chunk, std::shared_ptr<struct ChunkRenderData> & data)
{
	// Generate palette
	std::vector<utility::RGBA> & palette = data->palette;
	switch (setting->mode)
	{
	case Render::Mode::CHUNK:
	case Render::Mode::REGION:
	case Render::Mode::IMAGE:
	case Render::Mode::IMAGE_DIRECT:
		switch (chunk.getPaletteType())
		{
		case PaletteType::BLOCKID: fillPalette(chunk.getIDPalette(), setting->colors, palette); break;
		case PaletteType::NAMESPACEID: fillPalette(chunk.getNSPalette(), setting->colors, palette); break;
		default: return false;
		}
		return !palette.empty();
	default:
		return true;
	}
}

struct RegionRenderData
{
	int32_t x = 0, z = 0;
	std::vector<utility::RGBA> scratchRegion;
	std::vector<std::vector<utility::RGBA>> scratchImage;
};

RegionRender::RegionRender(std::shared_ptr<RenderSettings> _setting)
	: setting(_setting)
{
	chunks.reserve(REGION_COUNT * REGION_COUNT);
}

void RegionRender::add(const std::shared_ptr<ChunkRenderData> & data)
{
	if (!data)
		return;
	if (!data->palette.empty())
		chunks.emplace_back(data);
}

std::shared_ptr<RegionRenderData> RegionRender::draw(int x, int z)
{
	std::shared_ptr<RegionRenderData> data = std::make_shared<RegionRenderData>();

	data->x = x;
	data->z = z;

	if (setting->mode != Render::Mode::REGION_TINY && chunks.empty())
		return data;

	switch (setting->mode)
	{
	case Render::Mode::CHUNK:
		break;
	case Render::Mode::REGION:
		{
			std::vector<utility::RGBA> row(REGION_WIDTH);
			Image image(REGION_WIDTH, REGION_WIDTH);
			auto path = platform::path::join(setting->path, string::format("r.", x, ".", z, ".png"));
			// Generate "unsorted" chunks into scratch
			std::vector<std::vector<utility::RGBA>> scratch(CHUNK_COUNT);
			bool got_data = false;
			for (const auto & chunk : chunks)
			{
				// Was never run
				if (chunk->scratch.empty())
					continue;
				else
					got_data = true;
				auto pos = utility::ChunkPosition(chunk->x, chunk->z);
				// Move over the data, as it is now invalid
				scratch[utility::math::indexMod2d(REGION_COUNT, pos.x, pos.y)] = std::move(chunk->scratch);
			}

			if (!got_data)
				return data;

			// Save the image
			image.save(path, [&row, &scratch](uint32_t bz)
			{
				std::fill(row.begin(), row.end(), utility::RGBA());
				auto it = row.begin();
				for (int32_t cx = 0; cx < REGION_COUNT; ++cx)
				{
					auto cz = int32_t(bz / CHUNK_WIDTH);
					auto chunk = scratch[utility::math::index2d(REGION_COUNT, cx, cz)];
					if (chunk.empty())
					{
						std::advance(it, CHUNK_WIDTH);
						continue;
					}
					auto tileZ = bz % CHUNK_WIDTH;
					auto chunkBegin = chunk.begin();
					std::advance(chunkBegin, tileZ * CHUNK_WIDTH);
					auto chunkEnd = chunkBegin;
					std::advance(chunkEnd, CHUNK_WIDTH);
					it = std::copy(chunkBegin, chunkEnd, it);
				}
				return row;
			});
			setting->events.call(int(chunks.size()));
		}
		break;
	case Render::Mode::IMAGE:
		{
			// Merge chunks into one image
			data->scratchRegion.resize(REGION_WIDTH * REGION_WIDTH);
			for (const auto & chunk : chunks)
			{
				// Was never run
				if (chunk->scratch.empty())
					continue;
				auto pos = utility::ChunkPosition(chunk->x, chunk->z);
				auto intX = utility::math::mod(pos.x, REGION_COUNT);
				auto intZ = utility::math::mod(pos.y, REGION_COUNT);

				auto it = data->scratchRegion.begin();
				auto cit = chunk->scratch.begin();
				std::advance(it, CHUNK_WIDTH * REGION_WIDTH * intZ);
				auto preX = intX * CHUNK_WIDTH;
				auto postX = (REGION_COUNT - intX) * CHUNK_WIDTH;
				for (int iz = 0; iz < CHUNK_WIDTH; ++iz)
				{
					std::advance(it, preX);
					auto cend = cit;
					std::advance(cend, CHUNK_WIDTH);
					std::copy(cit, cend, it);
					std::advance(it, postX);
					std::advance(cit, CHUNK_WIDTH);
				}
			}
			setting->events.call(int(chunks.size()));
		}
		break;
	case Render::Mode::IMAGE_DIRECT:
		{
			// Put chunks into scratch image
			data->scratchImage.resize(CHUNK_COUNT);
			for (const auto & chunk : chunks)
			{
				if (chunk->scratch.empty())
					continue;
				auto pos = utility::ChunkPosition(chunk->x, chunk->z);
				// Swap data to improve performance
				std::swap(data->scratchImage[utility::math::indexMod2d(REGION_COUNT, pos.x, pos.y)], chunk->scratch);
			}
		}
		break;
	case Render::Mode::CHUNK_TINY:
		{
			data->scratchRegion.resize(CHUNK_COUNT);
			for (const auto & chunk : chunks)
			{
				if (chunk->scratch.empty())
					continue;
				auto pos = utility::ChunkPosition(chunk->x, chunk->z);
				// TODO: Test if this is legitimate
				auto cx = utility::math::mod(pos.x, REGION_COUNT);
				auto cz = utility::math::mod(pos.y, REGION_COUNT);

				data->scratchRegion[REGION_COUNT * cz + cx] = chunk->scratch[0];
			}
			setting->events.call(int(chunks.size()));
		}
		break;
	case Render::Mode::REGION_TINY:
		{
			data->scratchRegion.resize(1);
			data->scratchRegion[0] = utility::RGBA(255, 0, 0, 255);
		}
		break;
	}
	return data;
}

struct ImageRenderData
{
	AABB boundary;
};

WorldRender::WorldRender(std::shared_ptr<RenderSettings> _setting)
	: setting(_setting)
{
}

void WorldRender::add(std::shared_ptr<RegionRenderData> & data)
{
	if (!data->scratchImage.empty() || !data->scratchRegion.empty())
		regions.emplace(utility::PlanePosition(data->x, data->z), data);
}

void WorldRender::draw()
{
	if (regions.empty())
		return;

	auto data = std::make_shared<ImageRenderData>();

	switch (setting->mode)
	{
	case Render::Mode::CHUNK:
	case Render::Mode::REGION:
		break;
	case Render::Mode::IMAGE:
		{
			calculateBoundary(data);
			auto & boundary = data->boundary;
			auto width = 1 + boundary.bx - boundary.ax;
			auto height = 1 + boundary.bz - boundary.az;
			if (width <= 0 || height <= 0)
				break;
			auto offZ = boundary.az;
			std::vector<utility::RGBA> row(std::size_t(width * REGION_WIDTH));
			Image image(width * REGION_WIDTH, height * REGION_WIDTH);
			image.save(setting->path, [this, &boundary, &row, offZ](uint32_t bz)
			{
				std::fill(row.begin(), row.end(), utility::RGBA());
				auto rz = (int32_t(bz) / REGION_WIDTH) + offZ;
				auto rit = row.begin();
				for (int rx = boundary.ax; rx <= boundary.bx; ++rx, std::advance(rit, REGION_WIDTH))
				{
					auto regionit = regions.find({rx, rz});
					if (regionit == regions.end())
						continue;
					if (regionit->second->scratchRegion.empty())
						continue;
					auto it = regionit->second->scratchRegion.begin();
					std::advance(it, REGION_WIDTH * utility::math::mod(int32_t(bz), REGION_WIDTH));
					auto itend = it;
					std::advance(itend, REGION_WIDTH);
					std::copy(it, itend, rit);
				}
				return row;
			});
		}
		break;
	case Render::Mode::IMAGE_DIRECT:
		{
			calculateBoundary(data);
			auto & boundary = data->boundary;
			auto width = 1 + boundary.bx - boundary.ax;
			auto height = 1 + boundary.bz - boundary.az;
			if (width <= 0 || height <= 0)
				break;
			auto offZ = boundary.az;
			std::vector<utility::RGBA> row(std::size_t(width * REGION_WIDTH));
			Image image(width * REGION_WIDTH, height * REGION_WIDTH);
			image.save(setting->path, [this, &boundary, &row, offZ](uint32_t bz)
			{
				std::fill(row.begin(), row.end(), utility::RGBA());
				auto rz = (int32_t(bz) / REGION_WIDTH) + offZ;
				auto rit = row.begin();
				for (int rx = boundary.ax; rx <= boundary.bx; ++rx)
				{
					auto regionit = regions.find({rx, rz});
					if (regionit == regions.end())
					{
						std::advance(rit, REGION_WIDTH);
						continue;
					}

					int chunk_counter = 0;
					for (int i = 0; i < REGION_COUNT; ++i)
					{
						auto zRow = int32_t((bz / CHUNK_WIDTH) % REGION_COUNT);
						auto & chunk = regionit->second->scratchImage[utility::math::index2d(REGION_COUNT, i, zRow)];
						if (chunk.empty())
						{
							std::advance(rit, CHUNK_WIDTH);
							continue;
						}
						auto tileZ = bz % CHUNK_WIDTH;
						auto chunkBegin = chunk.begin();
						std::advance(chunkBegin, tileZ * CHUNK_WIDTH);
						auto chunkEnd = chunkBegin;
						std::advance(chunkEnd, CHUNK_WIDTH);
						rit = std::copy(chunkBegin, chunkEnd, rit);
						++chunk_counter;
					}
					if (bz % CHUNK_WIDTH == 0)
						setting->events.call(chunk_counter);
				}
				return row;
			});
		}
		break;
	case Render::Mode::CHUNK_TINY:
		{
			calculateBoundary(data);
			auto & boundary = data->boundary;
			auto width = 1 + boundary.bx - boundary.ax;
			auto height = 1 + boundary.bz - boundary.az;
			if (width <= 0 || height <= 0)
				break;
			auto offZ = boundary.az;

			std::vector<utility::RGBA> row(std::size_t(width * REGION_COUNT));
			Image image(width * REGION_COUNT, height * REGION_COUNT);
			image.save(setting->path, [this, &boundary, &row, offZ](uint32_t bz)
			{
				std::fill(row.begin(), row.end(), utility::RGBA());
				auto rz = int32_t(bz / REGION_COUNT) + offZ;
				auto rit = row.begin();
				for (int rx = boundary.ax; rx <= boundary.bx; ++rx, std::advance(rit, REGION_COUNT))
				{
					auto regionit = regions.find({rx, rz});
					if (regionit == regions.end())
						continue;
					auto it = regionit->second->scratchRegion.begin();
					std::advance(it, REGION_COUNT * utility::math::mod(int32_t(bz), REGION_COUNT));
					auto itend = it;
					std::advance(itend, REGION_COUNT);
					std::copy(it, itend, rit);
				}
				return row;
			});
		}
		break;
	case Render::Mode::REGION_TINY:
		{
			calculateBoundary(data);
			auto & boundary = data->boundary;
			auto width = 1 + boundary.bx - boundary.ax;
			auto height = 1 + boundary.bz - boundary.az;
			if (width <= 0 || height <= 0)
				break;
			auto offZ = boundary.az;

			std::vector<utility::RGBA> row((std::size_t(width)));
			Image image(width, height);
			image.save(setting->path, [this, &boundary, &row, offZ](uint32_t bz)
			{
				std::fill(row.begin(), row.end(), utility::RGBA());
				auto rz = int32_t(bz) + offZ;
				auto rit = row.begin();
				for (int rx = boundary.ax; rx <= boundary.bx; ++rx, ++rit)
				{
					auto regionit = regions.find({rx, rz});
					if (regionit == regions.end())
						continue;
					if (regionit->second->scratchRegion.empty())
						continue;
					*rit = regionit->second->scratchRegion[0];
				}
				return row;
			});
		}
		break;
	}
}

void WorldRender::eventRenderRegion(std::function<void(int)> && func)
{
	setting->events.add(std::move(func));
}

void WorldRender::calculateBoundary(std::shared_ptr<struct ImageRenderData> & data)
{
	auto & boundary = data->boundary;
	bool first = true;
	for (const auto & region : regions) {
		if (region.second->scratchRegion.empty())
			continue;
		auto rx = region.first.x, rz = region.first.y;
		if (first)
		{
			boundary.ax = boundary.bx = rx;
			boundary.az = boundary.bz = rz;
			first = false;
		}
		else
		{
			if (boundary.ax > rx) boundary.ax = rx;
			if (boundary.bx < rx) boundary.bx = rx;
			if (boundary.az > rz) boundary.az = rz;
			if (boundary.bz < rz) boundary.bz = rz;
		}
	}
}
