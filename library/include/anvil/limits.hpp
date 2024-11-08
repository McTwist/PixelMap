#pragma once
#ifndef ANVIL_LIMITS_HPP
#define ANVIL_LIMITS_HPP

#include "../limits.hpp"

constexpr int SECTION_Y = Minecraft::sectionHeight(Minecraft::SaveVersion::ANVIL);
constexpr uint32_t SECTIONS = Minecraft::sectionCount(Minecraft::SaveVersion::ANVIL);

constexpr uint32_t SECTION_SIZE = SECTION_X * SECTION_Z * SECTION_Y;

#endif // ANVIL_LIMITS_HPP
