#pragma once
#ifndef BEDROCK_V_HPP
#define BEDROCK_V_HPP

#include "format/leveldb.hpp"
#include "bedrock/world.hpp"
#include "render/utility.hpp"

namespace bedrock
{

class V : public LevelDB::Visitor
{
public:
	V(World & world);
    
    void visit(const std::vector<uint8_t> & key, const LevelDB::VectorData & data);
protected:
    World & world;
	// Namespace/Index translate table
	std::unordered_map<utility::ChunkPosition, std::unordered_map<std::string, uint16_t>> chunk_ns;
};

}

#endif // BEDROCK_V_HPP
