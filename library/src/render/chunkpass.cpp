#include "render/renderpass.hpp"

#include "render/renderpassdefine.hpp"
#include "render/image.hpp"
#include "render/render.hpp"
#include "blockcolor.hpp"
#include "chunk.hpp"
#include "platform.hpp"
#include "string.hpp"


using ChunkPassIntermediateFunction = std::function<void(const Chunk &, std::shared_ptr<struct ChunkRenderData>)>;

template<typename T>
void fillPalette(const std::vector<T> & src, const BlockColor & colors, std::vector<utility::RGBA> & dst)
{
	dst.resize(src.size());
	for (auto i = 0U; i < src.size(); ++i)
		dst[i] = colors.getColor(colors.getIndex(src[i]));
}

namespace ChunkPass
{

static ChunkPassIntermediateFunction ChunkBuild(std::shared_ptr<RenderSettings> setting, BlockPassFunction func)
{
	return [setting, func](const Chunk & chunk, std::shared_ptr<ChunkRenderData> data)
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
		::Image image(CHUNK_WIDTH, CHUNK_WIDTH);
		image.save(path, [&row, &palette, &chunk, func](uint32_t bz)
		{
			std::fill(row.begin(), row.end(), utility::RGBA());
			for (uint32_t bx = 0; bx < CHUNK_WIDTH; ++bx)
			{
				using namespace utility;
				BlockPassData passData{palette, chunk, Direction(0, -1, 0), Vector(bx, chunk.getMaxY(), bz), RGBA()};
				func(passData);
				row[bx] = passData.color;
			}
			return row;
		});
		setting->events.call(1);
	};
}

static ChunkPassIntermediateFunction RegionBuild(std::shared_ptr<RenderSettings> setting, BlockPassFunction)
{
	return [setting](const Chunk &, std::shared_ptr<ChunkRenderData>)
	{
		platform::path::mkdir(setting->path);
	};
}

static ChunkPassIntermediateFunction ImageBuild(std::shared_ptr<RenderSettings> setting, BlockPassFunction func)
{
	return [setting, func](const Chunk & chunk, std::shared_ptr<ChunkRenderData> data)
	{
		auto & area	= data->scratch;
		auto & palette = data->palette;
		area.resize(CHUNK_WIDTH * CHUNK_WIDTH);
		for (int32_t bz = 0; bz < CHUNK_WIDTH; ++bz)
		{
			for (int32_t bx = 0; bx < CHUNK_WIDTH; ++bx)
			{
				using namespace utility;
				BlockPassData passData{palette, chunk, Direction(0, -1, 0), Vector(bx, chunk.getMaxY(), bz), RGBA()};
				func(passData);
				area[utility::math::index2d(CHUNK_WIDTH, bx, bz)] = passData.color;
			}
		}
	};
}

static ChunkPassIntermediateFunction ChunkTinyBuild(std::shared_ptr<RenderSettings> setting, BlockPassFunction)
{
	return [setting](const Chunk &, std::shared_ptr<ChunkRenderData> data)
	{
		auto & area	= data->scratch;
		area.resize(1);
		area[0] = utility::RGBA(255, 0, 0, 255);
	};
}

}

ChunkPassFunction ChunkPassFactory::create(std::shared_ptr<RenderSettings> setting, BlockPassFunction func)
{
	std::vector<ChunkPassIntermediateFunction> pass;
	switch (setting->mode)
	{
	case Render::Mode::CHUNK:
		pass.emplace_back(ChunkPass::ChunkBuild(setting, func));
		break;
	case Render::Mode::REGION:
#ifdef ENABLE_WEBVIEW
	case Render::Mode::WEBVIEW:
#endif
		pass.emplace_back(ChunkPass::RegionBuild(setting, func));
		// Fall-through as it still needs to render each chunk
		[[fallthrough]];
	case Render::Mode::IMAGE:
	case Render::Mode::IMAGE_DIRECT:
		pass.emplace_back(ChunkPass::ImageBuild(setting, func));
		break;
	case Render::Mode::CHUNK_TINY:
		pass.emplace_back(ChunkPass::ChunkTinyBuild(setting, func));
		break;
	case Render::Mode::REGION_TINY:
		break;
	}
	auto generatePalette = [setting](const Chunk & chunk, std::shared_ptr<ChunkRenderData> & data)
	{
		// Generate palette
		std::vector<utility::RGBA> & palette = data->palette;
		switch (setting->mode)
		{
		case Render::Mode::CHUNK:
		case Render::Mode::REGION:
		case Render::Mode::IMAGE:
#ifdef ENABLE_WEBVIEW
		case Render::Mode::WEBVIEW:
#endif
		case Render::Mode::IMAGE_DIRECT:
			switch (chunk.getPaletteType())
			{
			case PaletteType::BLOCKID: fillPalette(chunk.getIDPalette(), setting->colors, palette); break;
			case PaletteType::NAMESPACEID: fillPalette(chunk.getNSPalette(), setting->colors, palette); break;
			default: return false;
			}
			return !palette.empty() && std::any_of(palette.begin(), palette.end(), [](const utility::RGBA & c) { return c.a > 0; });
		default:
			return true;
		}
	};
	return [passes{std::move(pass)}, generatePalette](const Chunk & chunk)
	{
		std::shared_ptr<struct ChunkRenderData> data = std::make_shared<ChunkRenderData>();
		if (!generatePalette(chunk, data))
			return data;

		data->x = chunk.getX();
		data->z = chunk.getZ();

		for (auto f : passes)
			f(chunk, data);
		return data;
	};
}

