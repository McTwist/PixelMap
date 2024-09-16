#pragma once
#ifndef ANVIL_V3_HPP
#define ANVIL_V3_HPP

#include "anvil/v.hpp"

#include <vector>
#include <array>

namespace anvil
{

class V3 : public V
{
public:
	V3(Chunk & chunk);

	bool visit(const NBT::Tag & tag);
private:
	// Block IDs
	std::vector<uint16_t> blocks;
	// Block ID/Index translate table
	std::array<uint16_t, 256 * 256> id;
	// State data
	int sections_left = 0;
};

}

#endif // ANVIL_V3_HPP
