#include "render/renderpass.hpp"

#include "render/renderpassdefine.hpp"
#include "render/image.hpp"
#include "render/render.hpp"
#include "platform.hpp"
#include "string.hpp"


using WorldPassIntermediateFunction = std::function<void(const std::unordered_map<utility::RegionPosition, std::shared_ptr<RegionRenderData>> &, std::shared_ptr<ImageRenderData> &)>;

namespace WorldPass
{

void calculateBoundary(const std::unordered_map<utility::RegionPosition, std::shared_ptr<RegionRenderData>> & regions, std::shared_ptr<ImageRenderData> & data)
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


static WorldPassIntermediateFunction ImageBuild(std::shared_ptr<RenderSettings> setting)
{
	return [setting](const std::unordered_map<utility::RegionPosition, std::shared_ptr<RegionRenderData>> & regions, std::shared_ptr<ImageRenderData> & data)
	{
		calculateBoundary(regions, data);
		auto & boundary = data->boundary;
		auto width = 1 + boundary.bx - boundary.ax;
		auto height = 1 + boundary.bz - boundary.az;
		if (width <= 0 || height <= 0)
			return;
		auto offZ = boundary.az;
		std::vector<utility::RGBA> row(std::size_t(width * REGION_WIDTH));
		Image image(width * REGION_WIDTH, height * REGION_WIDTH);
		image.save(setting->path, [&regions, &boundary, &row, offZ](uint32_t bz)
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
	};
}

static WorldPassIntermediateFunction ImageDirectBuild(std::shared_ptr<RenderSettings> setting)
{
	return [setting](const std::unordered_map<utility::RegionPosition, std::shared_ptr<RegionRenderData>> & regions, std::shared_ptr<ImageRenderData> & data)
	{
		calculateBoundary(regions, data);
		auto & boundary = data->boundary;
		auto width = 1 + boundary.bx - boundary.ax;
		auto height = 1 + boundary.bz - boundary.az;
		if (width <= 0 || height <= 0)
			return;
		auto offZ = boundary.az;
		std::vector<utility::RGBA> row(std::size_t(width * REGION_WIDTH));
		Image image(width * REGION_WIDTH, height * REGION_WIDTH);
		image.save(setting->path, [setting, &regions, &boundary, &row, offZ](uint32_t bz)
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
	};
}

static WorldPassIntermediateFunction ChunkTinyBuild(std::shared_ptr<RenderSettings> setting)
{
	return [setting](const std::unordered_map<utility::RegionPosition, std::shared_ptr<RegionRenderData>> & regions, std::shared_ptr<ImageRenderData> & data)
	{
		calculateBoundary(regions, data);
		auto & boundary = data->boundary;
		auto width = 1 + boundary.bx - boundary.ax;
		auto height = 1 + boundary.bz - boundary.az;
		if (width <= 0 || height <= 0)
			return;
		auto offZ = boundary.az;

		std::vector<utility::RGBA> row(std::size_t(width * REGION_COUNT));
		Image image(width * REGION_COUNT, height * REGION_COUNT);
		image.save(setting->path, [&regions, &boundary, &row, offZ](uint32_t bz)
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
	};
}

static WorldPassIntermediateFunction RegionTinyBuild(std::shared_ptr<RenderSettings> setting)
{
	return [setting](const std::unordered_map<utility::RegionPosition, std::shared_ptr<RegionRenderData>> & regions, std::shared_ptr<ImageRenderData> & data)
	{
		calculateBoundary(regions, data);
		auto & boundary = data->boundary;
		auto width = 1 + boundary.bx - boundary.ax;
		auto height = 1 + boundary.bz - boundary.az;
		if (width <= 0 || height <= 0)
			return;
		auto offZ = boundary.az;

		std::vector<utility::RGBA> row((std::size_t(width)));
		Image image(width, height);
		image.save(setting->path, [regions, &boundary, &row, offZ](uint32_t bz)
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
	};
}

}

WorldPassFunction WorldPassFactory::create(std::shared_ptr<RenderSettings> setting)
{
	std::vector<WorldPassIntermediateFunction> pass;
	switch (setting->mode)
	{
	case Render::Mode::CHUNK:
	case Render::Mode::REGION:
		break;
	case Render::Mode::IMAGE:
		pass.emplace_back(WorldPass::ImageBuild(setting));
		break;
	case Render::Mode::IMAGE_DIRECT:
		pass.emplace_back(WorldPass::ImageDirectBuild(setting));
		break;
	case Render::Mode::CHUNK_TINY:
		pass.emplace_back(WorldPass::ChunkTinyBuild(setting));
		break;
	case Render::Mode::REGION_TINY:
		pass.emplace_back(WorldPass::RegionTinyBuild(setting));
		break;
	}
	return [passes{std::move(pass)}](const std::unordered_map<utility::RegionPosition, std::shared_ptr<RegionRenderData>> & regions)
	{
		if (regions.empty())
			return;
	
		auto data = std::make_shared<ImageRenderData>();

		for (auto f : passes)
			f(regions, data);
	};
}

