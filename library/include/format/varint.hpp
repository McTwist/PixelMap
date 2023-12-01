#pragma once
#ifndef VARINT_HPP
#define VARINT_HPP

#include <cstdint>

namespace leveldb
{

/**
 * @brief Read a varint
 * Takes in a pointer to a buffer to iterate and unpack
 * an interger.
 * @tparam T Type to retrieve
 * @param ptr The pointer to iterate to read the integer
 * @return T The unpacked type
 */
template<typename T>
T read_varint(const uint8_t *& ptr);

/**
 * @brief Skip a varint
 * Takes in a pointer to a buffer to iterate and skip.
 * @param ptr The pointer to iterate to skip
 */
void skip_varint(const uint8_t *& ptr);


template<typename T>
T read_varint(const uint8_t *& ptr)
{
	T v = 0;
	// Try to avoid undefined behavior
	auto size = sizeof(T) * 8;
	for (auto i = 0U; i < size; i += 7, ++ptr)
	{
		v += T(*ptr & 127) << i;
		if ((*ptr & 128) == 0)
			break;
	}
	++ptr;
	return v;
}

inline void skip_varint(const uint8_t *& ptr)
{
	for (; (*ptr & 128) != 0; ++ptr) {}
	++ptr;
}

}

#endif // VARINT_HPP
