#include "render/render.hpp"

#include "render/renderpassdefine.hpp"

#include <algorithm>


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

std::shared_ptr<ChunkRenderData> ChunkRender::draw(ChunkPassFunction pass, const Chunk & chunk)
{
	return pass(chunk);
}

RegionRender::RegionRender()
{
	chunks.reserve(REGION_COUNT * REGION_COUNT);
}

void RegionRender::add(const std::shared_ptr<ChunkRenderData> & data)
{
	if (!data)
		return;
	if (!data->palette.empty() && !data->scratch.empty())
		chunks.emplace_back(data);
}

std::shared_ptr<RegionRenderData> RegionRender::draw(RegionPassFunction pass, int x, int z)
{
	return pass(x, z, chunks);
}

WorldRender::WorldRender(std::shared_ptr<RenderSettings> _setting)
	: setting(_setting)
{
}

void WorldRender::add(std::shared_ptr<RegionRenderData> & data)
{
	if (!data->scratchImage.empty() || !data->scratchRegion.empty())
		regions.emplace(utility::PlanePosition(data->x, data->z), data);
}

void WorldRender::draw(WorldPassFunction pass)
{
	pass(regions);
}

void WorldRender::eventRenderRegion(std::function<void(int)> && func)
{
	setting->event_chunkRender.add(std::move(func));
}

void WorldRender::eventTotalExtra(std::function<void(int)> && func)
{
	setting->event_extraTotal.add(std::move(func));
}

void WorldRender::eventRenderExtra(std::function<void(int)> && func)
{
	setting->event_extraAdd.add(std::move(func));
}

