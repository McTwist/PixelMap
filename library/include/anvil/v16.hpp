#pragma once
#ifndef ANVIL_V16_HPP
#define ANVIL_V16_HPP

#include "anvil/v.hpp"

namespace anvil
{

class V16 : public V
{
public:
	V16(Chunk & chunk) : V(chunk) {}

	bool visit(const NBT::Tag & tag) override;
private:
	// Block IDs
	std::vector<uint16_t> blocks;
	// Namespace Palette
	std::vector<std::string> palette;
	// Namespace/Index translate table
	std::unordered_map<std::string, uint16_t> ns;
	// State data
	int sections_left = 0;
	int palettes_left = 0;
	bool heightmaps = false;
};

}

#endif // ANVIL_V16_HPP
