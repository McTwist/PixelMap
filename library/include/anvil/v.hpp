#pragma once
#ifndef ANVIL_V_HPP
#define ANVIL_V_HPP

#include "format/nbt.hpp"
#include "chunk.hpp"

namespace anvil
{

class V : public NBT::Visitor
{
public:
	V(Chunk & chunk);
    
    bool visit(const NBT::Value & value);
    bool visit(const NBT::Tag & tag);
protected:
    Chunk & chunk;
    SectionData section;
};

}

#endif // ANVIL_V_HPP
