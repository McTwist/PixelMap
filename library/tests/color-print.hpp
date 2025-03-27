#pragma once

#include "render/color.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

namespace glm
{

// Overload to display values correctly for char
template<typename CTy, typename CTr>
std::basic_ostream<CTy,CTr>& operator<<(std::basic_ostream<CTy,CTr>& out, utility::RGBA const& g);
template<typename CTy, typename CTr>
std::basic_ostream<CTy,CTr>& operator<<(std::basic_ostream<CTy,CTr>& out, utility::RGBA const& g)
{
	return out << i16vec4(g);
}

}
