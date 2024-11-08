#pragma once
#ifndef ALPHA_LIMITS_HPP
#define ALPHA_LIMITS_HPP

#include "../limits.hpp"

constexpr int CHUNK_Y = Minecraft::worldHeight(Minecraft::SaveVersion::ALPHA);

constexpr uint32_t CHUNK_SIZE = SECTION_X * SECTION_Z * CHUNK_Y;

#endif // ALPHA_LIMITS_HPP
