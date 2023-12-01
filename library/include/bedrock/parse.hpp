#pragma once
#ifndef BEDROCK_PARSE_HPP
#define BEDROCK_PARSE_HPP

#include <vector>

#include <cstdint>

namespace parse
{
	struct ChunkKey {
		int32_t x, z;
		int32_t dimension = 0; // Optional
		uint8_t type;
		int8_t index = -1; // Optional
	};

	ChunkKey read_chunk_key(const std::vector<uint8_t> & key);

	namespace mc
	{
		bool is_key_sub_chunk_prefix(const std::vector<uint8_t> & key);
	}
} // namespace parse

#endif // BEDROCK_PARSE_HPP
