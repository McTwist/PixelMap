#pragma once
#ifndef PALETTE_HPP
#define PALETTE_HPP

#include "chunk.hpp"

#include <vector>
#include <array>
#include <string>
#include <unordered_map>
#include <cstdint>

namespace palette
{

constexpr std::size_t ID_SIZE = 256 * 256;

// ID palette
void translate(
		Chunk & chunk,
		SectionData && section,
		std::array<uint16_t, ID_SIZE> & id,
		std::vector<uint16_t> & blocks);

// Namespace palette
void translate(
		Chunk & chunk,
		SectionData && section,
		std::unordered_map<std::string, uint16_t> & ns,
		std::vector<uint16_t> & blocks,
		std::vector<std::string> && palette);

}

#endif // PALETTE_HPP
