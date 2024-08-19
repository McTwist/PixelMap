#include "render/renderpass.hpp"

#include "chunk.hpp"
#include "blockcolor.hpp"
#include "render/color.hpp"

namespace RenderPass
{

RenderPassFunction Default::build()
{
	using namespace utility;
	return [](RenderPassData & data)
	{
		auto p = space::to(data.pos);
		auto h = data.chunk.getHeight({p.x, p.z});
		data.pos.y = static_cast<Vector::value_type>(h - 1);
		if (data.pos.y < data.chunk.getMinY())
			return;
		auto tile = data.chunk.getTile(data.pos);
		data.color = data.palette[tile.index];
	};
}

RenderPassFunction Opaque::build()
{
	using namespace utility;
	return [](RenderPassData & data)
	{
		if (data.color.r > 0 || data.color.g > 0 || data.color.b > 0)
			data.color.a = 255;
	};
}

RenderPassFunction Heightmap::build()
{
	using namespace utility;
	return [](RenderPassData & data)
	{
		auto y = space::proj(data.pos.y, data.chunk.getMinY(), data.chunk.getMaxY(), 0, 255);
		data.color = color::blend(RGBA(0, 0, 0, 127), data.color, y);
	};
}

RenderPassFunction Gray::build()
{
	using namespace utility;
	return [](RenderPassData & data)
	{
		auto y = space::proj(data.pos.y, data.chunk.getMinY(), data.chunk.getMaxY(), 0, 255);
		data.color = utility::RGBA(y, y, y, 255);
	};
}

RenderPassFunction Color::build()
{
	using namespace utility;
	return [](RenderPassData & data)
	{
		static constexpr RGBA gradient[] =
		{
			RGBA(0x7F, 0x00, 0xFF, 0xFF), // light blue
			RGBA(0x00, 0x00, 0xFF, 0xFF), // blue
			RGBA(0x00, 0xFF, 0xFF, 0xFF), // cyan
			RGBA(0x00, 0xFF, 0x00, 0xFF), // green
			RGBA(0xFF, 0xFF, 0x00, 0xFF), // yellow
			RGBA(0xFF, 0x00, 0x00, 0xFF)  // red
		};

		auto t = space::proj(data.pos.y, data.chunk.getMinY(), data.chunk.getMaxY(), 0, 255);
		float step = 256.f / 5.0f;
		uint32_t bin = uint32_t(t / step);
		float norm = (t - bin*step) / step;

		RGBA block1 = gradient[bin];
		RGBA block2 = gradient[bin+1];
		data.color = color::interpolate(block1, block2, norm);
	};
}

Heightline::Heightline(int _frequency)
	: frequency(_frequency)
{
}

RenderPassFunction Heightline::build()
{
	using namespace utility;
	return [frequency{frequency}](RenderPassData & data)
	{
		(void)data.pos;
		if (frequency > 0 && space::to(data.pos).y % frequency == 0)
		{
			data.color = color::blend(RGBA(0, 0, 0, 128), data.color, 160);
		}
	};
}

RenderPassFunction Night::build()
{
	using namespace utility;
	return [](RenderPassData & data)
	{
		auto pos = data.pos - data.dir;
		auto tile = data.chunk.getTile(pos);
		float light = pow(0.9f, 15 - tile.blockLight);
		data.color = color::interpolate(data.color, RGBA(0, 0, 0, 255), 1 - light);
	};
}

Slice::Slice(int _y)
	: y(_y)
{}

RenderPassFunction Slice::build()
{
	using namespace utility;
	return [y{y}](RenderPassData & data)
	{
		auto pos = data.pos;
		if (pos.y > y)
			pos.y = float(y);
		auto tile = data.chunk.getTile(pos);
		data.color = data.palette[tile.index];
	};
}

RenderPassFunction Cave::build()
{
	using namespace utility;
	return [](RenderPassData & data)
	{
		bool a = true;
		RGBA c(0);
		glm::u8 prev = 0;
		auto ray = space::RayTracing(data.pos, data.dir);
		while ((c.a < 255 || a) && data.pos.y >= data.chunk.getMinY())
		{
			if (c.a > prev)
				prev = c.a;
			data.pos = ray.next();
			auto tile = data.chunk.getTile(data.pos);
			c = data.palette[tile.index];
			if (prev == 255 && c.a < 255)
				a = false;
		}
		data.color = c;
	};
}

Blend::Blend(Mode mode)
{
	using namespace utility::color;
	switch (mode)
	{
	case Mode::NORMAL: blend = normal; break;
	case Mode::MULTIPLY: blend = multiply; break;
	case Mode::SCREEN: blend = screen; break;
	case Mode::OVERLAY: blend = overlay; break;
	case Mode::DARKEN: blend = darken; break;
	case Mode::LIGHTEN: blend = lighten; break;
	case Mode::COLOR_DODGE: blend = color_dodge; break;
	case Mode::COLOR_BURN: blend = color_burn; break;
	case Mode::HARD_LIGHT: blend = hard_light; break;
	case Mode::SOFT_LIGHT: blend = soft_light; break;
	case Mode::DIFFERENCE: blend = difference; break;
	case Mode::EXCLUSION: blend = exclusion; break;
	case Mode::HUE: blend = hue; break;
	case Mode::SATURATION: blend = saturation; break;
	case Mode::COLOR: blend = color; break;
	case Mode::LUMINOSITY: blend = luminosity; break;
	case Mode::LEGACY:
	default:
		blend = [](auto a, auto b) { return utility::color::blend(b, a); };
		break;
	}
}

RenderPassFunction Blend::build()
{
	using namespace utility;
	return [blend{blend}](RenderPassData & data)
	{
		RGBA block = data.color;
		auto ray = space::RayTracing(data.pos, data.dir);
		for (RGBA curr = data.color;
			curr.a < 255 && data.pos.y >= data.chunk.getMinY() && data.pos.y <= data.chunk.getMaxY();
			data.pos = ray.next())
		{
			auto tile = data.chunk.getTile(data.pos);
			curr = data.palette[tile.index];
			block = blend(curr, block);
		}
		data.color = (data.pos.y < data.chunk.getMinY()) ? RGBA() : block;
	};
}

}
