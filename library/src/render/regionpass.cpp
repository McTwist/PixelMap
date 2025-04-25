#include "render/renderpass.hpp"

#include "render/renderpassdefine.hpp"
#include "render/image.hpp"
#include "render/render.hpp"
#include "platform.hpp"
#include "string.hpp"


using RegionPassIntermediateFunction = std::function<void(int x, int z, const std::vector<std::shared_ptr<ChunkRenderData>> &, std::shared_ptr<RegionRenderData> &)>;

namespace RegionPass
{

static RegionPassIntermediateFunction RegionBuild(std::shared_ptr<RenderSettings> setting)
{
	return [setting](int x, int z, const std::vector<std::shared_ptr<ChunkRenderData>> & chunks, std::shared_ptr<RegionRenderData> &)
	{
		if (chunks.empty())
			return;
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
			return;

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
	};
}

static RegionPassIntermediateFunction ImageBuild(std::shared_ptr<RenderSettings> setting)
{
	return [setting](int, int, const std::vector<std::shared_ptr<ChunkRenderData>> & chunks, std::shared_ptr<RegionRenderData> & data)
	{
		if (chunks.empty())
			return;
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
	};
}

static RegionPassIntermediateFunction ImageDirectBuild(std::shared_ptr<RenderSettings> setting)
{
	return [setting](int, int, const std::vector<std::shared_ptr<ChunkRenderData>> & chunks, std::shared_ptr<RegionRenderData> & data)
	{
		if (chunks.empty())
			return;
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
	};
}

static RegionPassIntermediateFunction ChunkTinyBuild(std::shared_ptr<RenderSettings> setting)
{
	return [setting](int, int, const std::vector<std::shared_ptr<ChunkRenderData>> & chunks, std::shared_ptr<RegionRenderData> & data)
	{
		if (chunks.empty())
			return;
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
	};
}

static RegionPassIntermediateFunction RegionTinyBuild(std::shared_ptr<RenderSettings> setting)
{
	return [setting](int, int, const std::vector<std::shared_ptr<ChunkRenderData>> &, std::shared_ptr<RegionRenderData> & data)
	{
		data->scratchRegion.resize(1);
		data->scratchRegion[0] = utility::RGBA(255, 0, 0, 255);
	};
}

}

RegionPassFunction RegionPassFactory::create(std::shared_ptr<RenderSettings> setting)
{
	std::vector<RegionPassIntermediateFunction> pass;
	switch (setting->mode)
	{
	case Render::Mode::CHUNK:
		break;
	case Render::Mode::REGION:
		pass.emplace_back(RegionPass::RegionBuild(setting));
		break;
	case Render::Mode::IMAGE:
		pass.emplace_back(RegionPass::ImageBuild(setting));
		break;
	case Render::Mode::IMAGE_DIRECT:
		pass.emplace_back(RegionPass::ImageDirectBuild(setting));
		break;
	case Render::Mode::CHUNK_TINY:
		pass.emplace_back(RegionPass::ChunkTinyBuild(setting));
		break;
	case Render::Mode::REGION_TINY:
		pass.emplace_back(RegionPass::RegionTinyBuild(setting));
		break;
	}
	return [passes{std::move(pass)}](int x, int z, const std::vector<std::shared_ptr<struct ChunkRenderData>> & chunks)
	{
		std::shared_ptr<RegionRenderData> data = std::make_shared<RegionRenderData>();
	
		data->x = x;
		data->z = z;

		for (auto f : passes)
			f(x, z, chunks, data);
		return data;
	};
}

