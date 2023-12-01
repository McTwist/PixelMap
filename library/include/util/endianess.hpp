#pragma once
#ifndef ENDIANESS_HPP
#define ENDIANESS_HPP

#include <cstdint>
#include <cassert>
#include <cstring>

namespace endianess
{
	/*
	 * Discover endianess
	 */
	inline bool isBig()
	{
		const int num = 1;
		return reinterpret_cast<const char &>(num) != 1;
	}
	inline bool isLittle()
	{
		const int num = 1;
		return reinterpret_cast<const char &>(num) == 1;
	}

	namespace internal
	{
		union Float
		{
			uint32_t i;
			float f;
		};
		union Double
		{
			uint64_t i;
			double f;
		};
	}

	/*
	 * From big endian
	 */
	template<typename T, class InputIt>
	inline T fromBig(InputIt src, std::size_t bytes)
	{
		assert(sizeof(T) >= bytes);
		assert(bytes >= 1);
		T dst = src[0];
		for (long i = 1; i < long(bytes); ++i)
			dst = (dst << 8) | src[i];
		return dst;
	}

	template<>
	inline float fromBig<>(uint8_t * src, std::size_t bytes)
	{
		assert(sizeof(float) == bytes);
		internal::Float dst;
		dst.i = fromBig<uint32_t>(src, bytes);
		return dst.f;
	}

	template<>
	inline double fromBig<>(uint8_t * src, std::size_t bytes)
	{
		assert(sizeof(double) == bytes);
		internal::Double dst;
		dst.i = fromBig<uint64_t>(src, bytes);
		return dst.f;
	}

	template<typename T, class InputIt, std::size_t bytes = sizeof(T)>
	inline T fromBig(InputIt src)
	{
		return fromBig<T>(src, bytes);
	}

	/*
		 * To big endian
	 */
	template<typename T, class OutputIt>
	inline void toBig(T src, OutputIt dst, std::size_t bytes)
	{
		assert(sizeof(T) >= bytes);
		assert(bytes >= 1);
		for (long i = long(bytes) - 1; i >= 0; --i, src >>= 8)
			dst[i] = src & 0xFF;
	}

	template<>
	inline void toBig(float src, uint8_t * dst, std::size_t bytes)
	{
		assert(sizeof(float) == bytes);
		internal::Float s;
		s.f = src;
		toBig<uint32_t>(s.i, dst, bytes);
	}

	template<>
	inline void toBig(double src, uint8_t * dst, std::size_t bytes)
	{
		assert(sizeof(double) == bytes);
		internal::Double s;
		s.f = src;
		toBig<uint64_t>(s.i, dst, bytes);
	}

	template<typename T, class OutputIt, std::size_t bytes = sizeof(T)>
	inline void toBig(T src, OutputIt dst)
	{
		toBig<T>(src, dst, bytes);
	}

	/*
	* From little endian
	*/
	template<typename T, class InputIt>
	inline T fromLittle(InputIt src, std::size_t bytes)
	{
		assert(sizeof(T) >= bytes);
		assert(bytes >= 1);
		T dst = src[bytes - 1];
		for (long i = long(bytes) - 2; i >= 0; --i)
			dst = (dst << 8) | src[i];
		return dst;
	}

	template<>
	inline float fromLittle<>(uint8_t * src, std::size_t bytes)
	{
		assert(sizeof(float) == bytes);
		internal::Float dst;
		dst.i = fromLittle<uint32_t>(src, bytes);
		return dst.f;
	}

	template<>
	inline double fromLittle<>(uint8_t * src, std::size_t bytes)
	{
		assert(sizeof(double) == bytes);
		internal::Double dst;
		dst.i = fromLittle<uint64_t>(src, bytes);
		return dst.f;
	}

	template<typename T, class InputIt, std::size_t bytes = sizeof(T)>
	inline T fromLittle(InputIt src)
	{
		return fromLittle<T>(src, bytes);
	}

	/*
	* To little endian
	*/
	template<typename T, class OutputIt>
	inline void toLittle(T src, OutputIt dst, std::size_t bytes)
	{
		assert(sizeof(T) >= bytes);
		assert(bytes >= 1);
		for (long i = 0U; i < long(bytes); ++i, src >>= 8)
			dst[i] = src & 0xFF;
	}

	template<>
	inline void toLittle<float, uint8_t *>(float src, uint8_t * dst, std::size_t bytes)
	{
		assert(sizeof(float) == bytes);
		internal::Float s;
		s.f = src;
		toLittle<uint32_t>(s.i, dst, bytes);
	}

	template<>
	inline void toLittle<double, uint8_t *>(double src, uint8_t * dst, std::size_t bytes)
	{
		assert(sizeof(double) == bytes);
		internal::Double s;
		s.f = src;
		toLittle<uint64_t>(s.i, dst, bytes);
	}

	template<typename T, class OutputIt, std::size_t bytes = sizeof(T)>
	inline void toLittle(T src, OutputIt dst)
	{
		toLittle<T>(src, dst, bytes);
	}
} // namespace endianess

#endif // ENDIANESS_HPP
