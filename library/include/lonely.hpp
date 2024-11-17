#pragma once
#ifndef LONELY_HPP

#include "render/utility.hpp"

#include <unordered_set>
#include <memory>

// TODO: Remove unnecessary dependency, making it more standalone
// Note: May find some better way to iterate through a region
namespace region
{
class RegionFile;
struct ChunkData;
}

/**
 * @brief Locates lonely chunks
 */
class Lonely
{
	typedef std::unordered_set<utility::PlanePosition> PlaneMap;
public:

	/**
	 * @brief Locate lonely chunks
	 * @param region The region to check
	 */
	void locate(const std::shared_ptr<const region::RegionFile> & region);

	/**
	 * @brief Locate a specific position
	 * @param pos The position to check
	 */
	void locate(const utility::PlanePosition & pos);
	/**
	 * @brief Process gathered data
	 * Depending on mode, this will iterate through all chunks and locate the lonely ones
	 */
	void process();

	/**
	 * @brief Find out of a whole region only consist of lonely chunks
	 * @param region The region to check
	 * @return True if lonely, false otherwise
	 */
	bool isLonely(const std::shared_ptr<const region::RegionFile> & region) const;
	/**
	 * @brief Find out if a chunk is lonely
	 * @param chunk The chunk to check
	 * @return True if lonely, false otherwise
	 */
	bool isLonely(const std::shared_ptr<const region::ChunkData> & chunk) const;

	/**
	 * @brief Find out if a position is lonely
	 * @param pos The position to check
	 * @return True if lonely, false otherwise
	 */
	bool isLonely(const utility::PlanePosition & pos) const;

private:
	// Version 1 & 2
	PlaneMap regions;
	// Version 1 & 3
	PlaneMap chunks;
	PlaneMap known_chunks;
};

#endif // LONELY_HPP
