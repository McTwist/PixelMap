#include "render/renderpassdefine.hpp"

std::vector<utility::RGBA> RenderPass::shrinkRegion(const std::vector<utility::RGBA> & from)
{
	auto size = from.size();
	std::size_t width = std::sqrt(size);
	std::vector<utility::RGBA> scratch(size / 4);
	for (auto z = 0U; z < width; z += 2)
	{
		for (auto x = 0U; x < width; x += 2)
		{
			scratch[utility::math::index2d(width / 2, x / 2, z / 2)] =
				utility::color::lerp(
					utility::color::lerp(
						from[utility::math::index2d(width, x, z)],
						from[utility::math::index2d(width, x+1, z)],
						.5f),
					utility::color::lerp(
						from[utility::math::index2d(width, x, z+1)],
						from[utility::math::index2d(width, x+1, z+1)],
						.5f),
					.5f);
		}
	}
	return scratch;
}
