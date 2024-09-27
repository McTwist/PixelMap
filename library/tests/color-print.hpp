#pragma once

#include "render/color.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/io.hpp>

// Overload to display values correctly for char
template<typename CTy, typename CTr>
std::basic_ostream<CTy,CTr>& glm::operator<<(std::basic_ostream<CTy,CTr>& out, utility::RGBA const& g)
{
	return out << glm::i16vec4(g);
}
