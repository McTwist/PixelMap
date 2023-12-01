#pragma once
#ifndef LIMITS_HPP
#define LIMITS_HPP

#include "minecraft.hpp"

#include <limits>

constexpr int SECTION_X = Minecraft::chunkWidth();
constexpr int SECTION_Z = Minecraft::chunkWidth();

constexpr uint32_t SECTION_AREA = SECTION_X * SECTION_Z;

constexpr auto BLOCK_ID_MAX = std::numeric_limits<uint16_t>::max();

#endif // LIMITS_HPP
