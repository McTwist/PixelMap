#pragma once
#ifndef NIBBLE_HPP
#define NIBBLE_HPP

#include "vectorview.hpp"

#include <vector>
#include <type_traits>

/**
 * @brief Take 4 bits from data
 * @param data The data to get the nibble from
 * @param index The internal index of 4 bits
 * @return The value to retrieve from
 */
template<typename T>
T nibble4(const std::vector<T> & data, std::size_t index);
template<typename T>
T nibble4(const VectorView<T> & data, std::size_t index);
/**
 * @brief Copy 4 nibbles over to easier handled types
 * @param src The source data
 * @param dst The destination
 */
template<typename T, typename D>
void nibble4Copy(const std::vector<T> & src, std::vector<D> & dst);
template<typename T, typename D>
void nibble4Copy(const std::vector<T> & src, VectorView<D> & dst);
template<typename T, typename D>
void nibble4Copy(const VectorView<T> & src, std::vector<D> & dst);
template<typename T, typename D>
void nibble4Copy(const VectorView<T> & src, VectorView<D> & dst);

namespace MC13
{

/**
 * @brief Get nibble of specific bits from any type
 * @param data The data to get nibble from
 * @param index The internal index from the bits and type used
 * @param bits The amount of bits between each nibble
 * @return The nibble from the speccific index
 */
template<typename T>
T nibble(const std::vector<T> & data, std::size_t index, const std::size_t bits);
template<typename T>
T nibble(const VectorView<T> & data, std::size_t index, const std::size_t bits);
/**
 * @brief Get nibble of specific bits from any type
 * @param data The data to get nibble from
 * @param index The internal index from the bits and type used
 * @return The nibble from the speccific index
 */
template<typename T, const std::size_t bits>
T nibble(const std::vector<T> & data, std::size_t index);
template<typename T, const std::size_t bits>
T nibble(const VectorView<T> & data, std::size_t index);
/**
 * @brief Copy nibbles over to easier handled types
 * @param src The source data
 * @param dst The destination
 * @param bits The amount of bits between each nibble
 */
template<typename T, typename D>
void nibbleCopy(const std::vector<T> & src, std::vector<D> & dst, const std::size_t bits);
template<typename T, typename D>
void nibbleCopy(const std::vector<T> & src, VectorView<D> & dst, const std::size_t bits);
template<typename T, typename D>
void nibbleCopy(const VectorView<T> & src, std::vector<D> & dst, const std::size_t bits);
template<typename T, typename D>
void nibbleCopy(const VectorView<T> & src, VectorView<D> & dst, const std::size_t bits);
/**
 * @brief Copy nibbles over to easier handled types
 * @param src The source data
 * @param dst The destination
 */
template<typename T, typename D, const std::size_t bits>
void nibbleCopy(const std::vector<T> & src, std::vector<D> & dst);
template<typename T, typename D, const std::size_t bits>
void nibbleCopy(const std::vector<T> & src, VectorView<D> & dst);
template<typename T, typename D, const std::size_t bits>
void nibbleCopy(const VectorView<T> & src, std::vector<D> & dst);
template<typename T, typename D, const std::size_t bits>
void nibbleCopy(const VectorView<T> & src, VectorView<D> & dst);

}

namespace MC16
{

/**
 * @brief Get nibble of specific bits from any type
 * @param data The data to get nibble from
 * @param index The internal index from the bits and type used
 * @param bits The amount of bits between each nibble
 * @return The nibble from the speccific index
 */
template<typename T>
T nibble(const std::vector<T> & data, std::size_t index, const std::size_t bits);
template<typename T>
T nibble(const VectorView<T> & data, std::size_t index, const std::size_t bits);
/**
 * @brief Get nibble of specific bits from any type
 * @param data The data to get nibble from
 * @param index The internal index from the bits and type used
 * @return The nibble from the speccific index
 */
template<typename T, const std::size_t bits>
T nibble(const std::vector<T> & data, std::size_t index);
template<typename T, const std::size_t bits>
T nibble(const VectorView<T> & data, std::size_t index);
/**
 * @brief Copy nibbles over to easier handled types
 * @param src The source data
 * @param dst The destination
 * @param bits The amount of bits between each nibble
 */
template<typename T, typename D>
void nibbleCopy(const std::vector<T> & src, std::vector<D> & dst, const std::size_t bits);
template<typename T, typename D>
void nibbleCopy(const std::vector<T> & src, VectorView<D> & dst, const std::size_t bits);
template<typename T, typename D>
void nibbleCopy(const VectorView<T> & src, std::vector<D> & dst, const std::size_t bits);
template<typename T, typename D>
void nibbleCopy(const VectorView<T> & src, VectorView<D> & dst, const std::size_t bits);
/**
 * @brief Copy nibbles over to easier handled types
 * @param src The source data
 * @param dst The destination
 */
template<typename T, typename D, const std::size_t bits>
void nibbleCopy(const std::vector<T> & src, std::vector<D> & dst);
template<typename T, typename D, const std::size_t bits>
void nibbleCopy(const std::vector<T> & src, VectorView<D> & dst);
template<typename T, typename D, const std::size_t bits>
void nibbleCopy(const VectorView<T> & src, std::vector<D> & dst);
template<typename T, typename D, const std::size_t bits>
void nibbleCopy(const VectorView<T> & src, VectorView<D> & dst);

}

/*
 * Implementation
 */

template<typename T>
inline T nibble4(const std::vector<T> & data, std::size_t index)
{
	return nibble4(VectorView<T>{const_cast<T *>(data.data()), data.size()}, index);
}
template<typename T>
inline T nibble4(const VectorView<T> & data, std::size_t index)
{
	// Note: Premature optimization is the root to all evil
	//return (data[index >> 1] >> ((index & 1) << 2)) & 0x0F;
	return (index % 2 == 0 ? data[index >> 1] : (data[index >> 1] >> 4)) & 0x0F;
}

template<typename T, typename D>
inline void nibble4Copy(const std::vector<T> & src, std::vector<D> & dst)
{
	auto _dst = VectorView<D>{dst.data(), dst.size()};
	nibble4Copy(VectorView<T>{const_cast<T *>(src.data()), src.size()}, _dst);
}
template<typename T, typename D>
inline void nibble4Copy(const std::vector<T> & src, VectorView<D> & dst)
{
	nibble4Copy(VectorView<T>{const_cast<T *>(src.data()), src.size()}, dst);
}
template<typename T, typename D>
inline void nibble4Copy(const VectorView<T> & src, std::vector<D> & dst)
{
	auto _dst = VectorView<D>{dst.data(), dst.size()};
	nibble4Copy(src, _dst);
}
template<typename T, typename D>
inline void nibble4Copy(const VectorView<T> & src, VectorView<D> & dst)
{
	auto s = src.size() << 1;
	for (auto i = 0U; i < dst.size() && i < s; ++i)
		dst[i] = D(nibble4(src, i));
}

/*
 * MC13
 */

template<typename T>
inline T MC13::nibble(const std::vector<T> & data, std::size_t index, const std::size_t bits)
{
	return nibble(VectorView<T>{const_cast<T *>(data.data()), data.size()}, index, bits);
}
template<typename T>
T MC13::nibble(const VectorView<T> & data, std::size_t index, const std::size_t bits)
{
	const auto size = sizeof(T) << 3;
	// Same size, no need to do anything else
	if (bits == size)
	{
		return data[index];
	}
	using MaskType = typename std::make_unsigned<T>::type;
	const MaskType mask = -1;
	const MaskType realMask = mask >> (size - bits);
	std::size_t start = index * bits;
	std::size_t i = start / (8 * sizeof(T));
	std::size_t b = start & (size - 1);

	/**
	 * Creates a mask for previous type, which is then indexed to specifed
	 * location.
	 */
#define CREATE_INDEXED_MASK(I) MaskType(realMask << (I))
#define CREATE_INDEXED_MASK_H(I) MaskType(realMask >> (I))

	// May read only one part
	if (b + bits <= size)
	{
		return T((CREATE_INDEXED_MASK(b) & MaskType(data[i])) >> b);
	}
	// May need	up to two parts
	else if (bits <= size)
	{
		MaskType lower = (CREATE_INDEXED_MASK(b) & MaskType(data[i])) >> b;
		auto offset = size - b;
		MaskType higher = (CREATE_INDEXED_MASK_H(offset) & MaskType(data[i+1])) << offset;
		return T(lower | higher);
	}
	// Spans over 2 or more parts

	/*
	 * Deliberately left out, as when this happens, the storage type is
	 * either wrong or we got computer supporting more than 64-bit natively.
	 * Might implement it in the future whenever found it it is needed.
	 * For now, all Minecraft tags that includes this kind of structure
	 * is stored in long arrays. This results in the possibility of
	 * 2^63-1 amount of integers before this problem will occur.
	 * Also, highest amount of index stored in a section is 4095.
	 */
	return 0;
#undef CREATE_INDEXED_MASK_H
#undef CREATE_INDEXED_MASK
}

template<typename T, const std::size_t bits>
inline T MC13::nibble(const std::vector<T> & data, std::size_t index)
{
	return nibble(VectorView<T>{const_cast<T *>(data.data()), data.size()}, index);
}
template<typename T, const std::size_t bits>
inline T MC13::nibble(const VectorView<T> & data, std::size_t index)
{
	return nibble(data, index, bits);
}

template<typename T, typename D>
inline void MC13::nibbleCopy(const std::vector<T> & src, std::vector<D> & dst, const std::size_t bits)
{
	auto _dst = VectorView<D>{dst.data(), dst.size()};
	nibbleCopy(VectorView<T>{const_cast<T *>(src.data()), src.size()}, _dst, bits);
}
template<typename T, typename D>
inline void MC13::nibbleCopy(const std::vector<T> & src, VectorView<D> & dst, const std::size_t bits)
{
	nibbleCopy(VectorView<T>{const_cast<T *>(src.data()), src.size()}, dst, bits);
}
template<typename T, typename D>
inline void MC13::nibbleCopy(const VectorView<T> & src, std::vector<D> & dst, const std::size_t bits)
{
	auto _dst = VectorView<D>{dst.data(), dst.size()};
	nibbleCopy(src, _dst, bits);
}
template<typename T, typename D>
inline void MC13::nibbleCopy(const VectorView<T> & src, VectorView<D> & dst, const std::size_t bits)
{
	auto s = (src.size() * (sizeof(T) << 3)) / bits;
	for (auto i = 0U; i < dst.size() && i < s; ++i)
		dst[i] = D(nibble(src, i, bits));
}

template<typename T, typename D, const std::size_t bits>
inline void MC13::nibbleCopy(const std::vector<T> & src, std::vector<D> & dst)
{
	auto _dst = VectorView<D>{dst.data(), dst.size()};
	nibbleCopy(VectorView<T>{const_cast<T *>(src.data()), src.size()}, _dst, bits);
}
template<typename T, typename D, const std::size_t bits>
inline void MC13::nibbleCopy(const std::vector<T> & src, VectorView<D> & dst)
{
	nibbleCopy(VectorView<T>{const_cast<T *>(src.data()), src.size()}, dst, bits);
}
template<typename T, typename D, const std::size_t bits>
inline void MC13::nibbleCopy(const VectorView<T> & src, std::vector<D> & dst)
{
	auto _dst = VectorView<D>{dst.data(), dst.size()};
	nibbleCopy(src, _dst, bits);
}
template<typename T, typename D, const std::size_t bits>
inline void MC13::nibbleCopy(const VectorView<T> & src, VectorView<D> & dst)
{
	nibbleCopy(src, dst, bits);
}

/*
 * MC16
 */

template<typename T>
inline T MC16::nibble(const std::vector<T> & data, std::size_t index, const std::size_t bits)
{
	return nibble(VectorView<T>{const_cast<T *>(data.data()), data.size()}, index, bits);
}
template<typename T>
T MC16::nibble(const VectorView<T> & data, std::size_t index, const std::size_t bits)
{
	const auto size = sizeof(T) << 3;
	// Same size, no need to do anything else
	if (bits == size)
	{
		return data[index];
	}
	typedef typename std::make_unsigned<T>::type MaskType;
	const MaskType mask = -1;
	const MaskType realMask = mask >> (size - bits);
	std::size_t parts = size / bits;
	std::size_t i = index / parts;
	std::size_t b = (index % parts) * bits;

	/**
	 * Creates a mask for previous type, which is then indexed to specifed
	 * location.
	 */
	return (MaskType(data[i]) >> b) & realMask;
}

template<typename T, const std::size_t bits>
inline T MC16::nibble(const std::vector<T> & data, std::size_t index)
{
	return nibble(VectorView<T>{const_cast<T *>(data.data()), data.size()}, index, bits);
}
template<typename T, const std::size_t bits>
inline T MC16::nibble(const VectorView<T> & data, std::size_t index)
{
	return nibble(data, index, bits);
}

template<typename T, typename D>
inline void MC16::nibbleCopy(const std::vector<T> & src, std::vector<D> & dst, const std::size_t bits)
{
	auto _dst = VectorView<D>{dst.data(), dst.size()};
	nibbleCopy(VectorView<T>{const_cast<T *>(src.data()), src.size()}, _dst, bits);
}
template<typename T, typename D>
inline void MC16::nibbleCopy(const std::vector<T> & src, VectorView<D> & dst, const std::size_t bits)
{
	nibbleCopy(VectorView<T>{const_cast<T *>(src.data()), src.size()}, dst, bits);
}
template<typename T, typename D>
inline void MC16::nibbleCopy(const VectorView<T> & src, std::vector<D> & dst, const std::size_t bits)
{
	auto _dst = VectorView<D>{dst.data(), dst.size()};
	nibbleCopy(src, _dst, bits);
}
template<typename T, typename D>
inline void MC16::nibbleCopy(const VectorView<T> & src, VectorView<D> & dst, const std::size_t bits)
{
	auto s = (src.size() * (sizeof(T) << 3)) / bits;
	for (auto i = 0U; i < dst.size() && i < s; ++i)
		dst[i] = D(nibble(src, i, bits));
}

template<typename T, typename D, const std::size_t bits>
inline void MC16::nibbleCopy(const std::vector<T> & src, std::vector<D> & dst)
{
	auto _dst = VectorView<D>{dst.data(), dst.size()};
	nibbleCopy(VectorView<T>{const_cast<T *>(src.data()), src.size()}, _dst, bits);
}
template<typename T, typename D, const std::size_t bits>
inline void MC16::nibbleCopy(const std::vector<T> & src, VectorView<D> & dst)
{
	nibbleCopy(VectorView<T>{const_cast<T *>(src.data()), src.size()}, dst, bits);
}
template<typename T, typename D, const std::size_t bits>
inline void MC16::nibbleCopy(const VectorView<T> & src, std::vector<D> & dst)
{
	auto _dst = VectorView<D>{dst.data(), dst.size()};
	nibbleCopy(src, _dst, bits);
}
template<typename T, typename D, const std::size_t bits>
inline void MC16::nibbleCopy(const VectorView<T> & src, VectorView<D> & dst)
{
	nibbleCopy(src, dst, bits);
}

#endif // NIBBLE_HPP
