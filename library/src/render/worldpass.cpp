#include "render/renderpass.hpp"

#include "render/renderpassdefine.hpp"
#include "render/image.hpp"
#include "render/render.hpp"
#ifdef ENABLE_WEBVIEW
#include "util/webview.hpp"
#endif
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

#ifdef ENABLE_WEBVIEW
static WorldPassIntermediateFunction WebViewBuild(std::shared_ptr<RenderSettings> setting)
{
	return [setting](const std::unordered_map<utility::RegionPosition, std::shared_ptr<RegionRenderData>> & regions, std::shared_ptr<ImageRenderData> & data)
	{
		WebView::createDefaultRoot(setting->path);
		calculateBoundary(regions, data);
		auto & boundary = data->boundary;
		auto width = 1 + boundary.bx - boundary.ax;
		auto height = 1 + boundary.bz - boundary.az;
		if (width <= 0 || height <= 0)
			return;
		std::vector<utility::RGBA> row((std::size_t)(REGION_WIDTH));
		Image image(REGION_WIDTH, REGION_WIDTH);
		for (int zoom = 7; zoom > 0; --zoom)
		{
			auto zoomLevel = 8 - zoom;
			int steps = 1 << zoomLevel;
			uint32_t size = REGION_WIDTH / steps;
			auto zoom_path = WebView::getRegionFolder(setting->path, zoom);
			platform::path::mkdir(zoom_path);
			// Note: This is inefficient for most worlds
			// TODO: Create set of zoomed coordinates, add all regions zoomed to it,
			// reducing the amount of iterations significantly
			auto topLeft = utility::coord::regionZoom({boundary.ax, boundary.az}, zoomLevel);
			auto botRight = utility::coord::regionZoom({boundary.bx, boundary.bz}, zoomLevel);
			for (int z = topLeft.y; z <= botRight.y; ++z)
			{
				for (int x = topLeft.x; x <= botRight.x; ++x)
				{
					auto path = platform::path::join(zoom_path, string::format("r.", x, ".", z, ".png"));
					auto zz = z * steps;
					auto xx = x * steps;
					image.save(path, [&regions, &row, zz, xx, size, steps](uint32_t bz)
					{
						std::fill(row.begin(), row.end(), utility::RGBA());
						auto rit = row.begin();
						int rz = zz + int32_t(bz / size);
						for (int rx = xx; rx < xx + steps; ++rx, std::advance(rit, size))
						{
							auto regionit = regions.find({rx, rz});
							if (regionit == regions.end())
								continue;
							if (regionit->second->scratchRegion.empty())
								continue;
							auto it = regionit->second->scratchRegion.begin();
							std::advance(it, size * utility::math::mod(int32_t(bz), size));
							auto itend = it;
							std::advance(itend, size);
							std::copy(it, itend, rit);
						}
						return row;
					});
					// Shrink regions for next step
					if (zoom > 1)
					{
						for (int rz = zz; rz < zz + steps; ++rz)
						{
							for (int rx = xx; rx < xx + steps; ++rx)
							{
								auto regionit = regions.find({rx, rz});
								if (regionit == regions.end())
									continue;
								if (regionit->second->scratchRegion.empty())
									continue;

								regionit->second->scratchRegion = std::move(RenderPass::shrinkRegion(regionit->second->scratchRegion));
							}
						}
					}
				}
			}
		}
	};
}
#endif

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
#ifdef ENABLE_WEBVIEW
	case Render::Mode::WEBVIEW:
		pass.emplace_back(WorldPass::WebViewBuild(setting));
		break;
#endif
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

