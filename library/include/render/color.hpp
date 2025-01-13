#pragma once
#ifndef COLOR_HPP
#define COLOR_HPP

#include <glm/glm.hpp>

namespace utility
{
	using RGBA = glm::u8vec4;

	namespace color
	{
		RGBA normal(RGBA background, RGBA foreground);
		RGBA multiply(RGBA background, RGBA foreground);
		RGBA screen(RGBA background, RGBA foreground);
		RGBA overlay(RGBA background, RGBA foreground);
		RGBA darken(RGBA background, RGBA foreground);
		RGBA lighten(RGBA background, RGBA foreground);
		RGBA color_dodge(RGBA background, RGBA foreground);
		RGBA color_burn(RGBA background, RGBA foreground);
		RGBA hard_light(RGBA background, RGBA foreground);
		RGBA soft_light(RGBA background, RGBA foreground);
		RGBA difference(RGBA background, RGBA foreground);
		RGBA exclusion(RGBA background, RGBA foreground);
		RGBA hue(RGBA background, RGBA foreground);
		RGBA saturation(RGBA background, RGBA foreground);
		RGBA color(RGBA background, RGBA foreground);
		RGBA luminosity(RGBA background, RGBA foreground);
	}
}

#endif // COLOR_HPP
