#pragma once
#ifndef ALPHA_V_HPP
#define ALPHA_V_HPP

#include "format/nbt.hpp"
#include "chunk.hpp"

namespace alpha
{

class V : public NBT::Visitor
{
public:
	V(Chunk & chunk);
    
    bool visit(const NBT::Value & value);
    bool visit(const NBT::Tag & tag);
protected:
    Chunk & chunk;
    SectionData sections[8];
	// Block IDs
	std::vector<uint16_t> blocks[8];
	// Block ID/Index translate table
	std::array<uint16_t, 256 * 256> id;
	bool level = false;
};

}

#endif // ALPHA_V_HPP
