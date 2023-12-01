#include "bedrock/parse.hpp"

#include "util/endianess.hpp"

namespace parse
{

ChunkKey read_chunk_key(const std::vector<uint8_t> & key)
{
	/*
	= Key value =
	chunkX: int32
	chunkZ: int32
	(dimension: int32) // Non-existant when dimension == 0
	type: int8
	(index: int8) // SubChunkPrefix
	?: int8 // 0 if type == 51 or 57, otherwise 1
	?: int8 // There is some sort of iteration
	timestamp?: int16 // Values change rarely
	PADDING: int32 = 0 // New since version 9
	*/
	ChunkKey chunk_key;
	auto ptr = key.data();
	chunk_key.x = endianess::fromLittle<int32_t>(ptr);
	ptr += sizeof(int32_t);
	chunk_key.z = endianess::fromLittle<int32_t>(ptr);
	ptr += sizeof(int32_t);
	if (key.size() == 13 || key.size() == 14
		|| key.size() == 21 || key.size() == 22)
	{
		chunk_key.dimension = endianess::fromLittle<int32_t>(ptr);
		ptr += sizeof(int32_t);
	}
	chunk_key.type = *(ptr++);
	chunk_key.index = (key.size() % 2 == 0) ? *(ptr++) : -1;
	return chunk_key;
}

namespace mc
{

bool is_key_sub_chunk_prefix(const std::vector<uint8_t> & key)
{
	return key.size() != 9 && key.size() != 10
		&& key.size() != 13 && key.size() != 14
		&& key.size() != 17 && key.size() != 18
		&& key.size() != 21 && key.size() != 22;
}

} // namespace mc

} // namespace parse
