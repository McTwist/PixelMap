#pragma once
#ifndef BEDROCK_LIMITS_HPP
#define BEDROCK_LIMITS_HPP

#include "../limits.hpp"

constexpr int SECTION_Y = Minecraft::sectionHeight(Minecraft::SaveVersion::LEVELDB);
constexpr uint32_t SECTIONS = Minecraft::sectionCount(Minecraft::SaveVersion::LEVELDB);

constexpr uint32_t SECTION_SIZE = SECTION_X * SECTION_Z * SECTION_Y;

#endif // BEDROCK_LIMITS_HPP
