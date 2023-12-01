#pragma once
#ifndef ANVIL_V18_HPP
#define ANVIL_V18_HPP

#include "anvil/v.hpp"

namespace anvil
{

class V18 : public V
{
public:
	V18(Chunk & chunk) : V(chunk) {}
	bool visit(const NBT::Tag & tag);
private:
	// Block IDs
	//std::vector<uint16_t> blocks;
	NBT::NBTLongArray blocks;
	// Namespace Palette
	std::vector<std::string> palette;
	// Namespace/Index translate table
	std::unordered_map<std::string, uint16_t> ns;
	// State data
	int sections_left = 0;
	int palettes_left = 0;
	bool heightmaps = false, block_states = false;
};

}

#endif // ANVIL_V18_HPP
