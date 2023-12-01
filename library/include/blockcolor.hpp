#pragma once
#ifndef BLOCK_COLOR_HPP
#define BLOCK_COLOR_HPP

#include "render/utility.hpp"

#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>


/**
 * @brief Keeps track of block colors
 * Handles colors for blocks. Also includes a file format to store block
 * colors that can be shared so profiles could be set up.
 */
class BlockColor
{
public:
	typedef uint32_t ColorIndex;

	/**
	 * @brief Reads a file
	 * @param file The file path
	 * @return False if errors, true otherwise
	 */
	bool read(const std::string & file = {});

	/**
	 * @brief Get index by block id
	 * @param id Block ID
	 * @return A color index to be used in a palette
	 */
	ColorIndex getIndex(uint16_t id) const;

	/**
	 * @brief Get index by namespace id
	 * @param id Namespace ID
	 * @return  A color index to be used in a palette
	 */
	ColorIndex getIndex(const std::string & id) const;

	/**
	 * @brief Get color by index
	 * @param index An index for a specific color
	 * @return A color for the index
	 */
	utility::RGBA getColor(ColorIndex index) const;

	/**
	 * @brief Colors exists
	 * @return true Colors exist
	 * @return false No colors
	 */
	bool hasColors() const;

private:

	// The colors stored away
	std::unordered_map<uint16_t, ColorIndex> old_indices;
	std::unordered_map<std::string, ColorIndex> new_indices;
	std::vector<utility::RGBA> colors;
};

#endif // BLOCK_COLOR_HPP
