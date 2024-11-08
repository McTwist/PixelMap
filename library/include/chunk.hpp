#pragma once
#ifndef CHUNK_HPP
#define CHUNK_HPP

#include "render/utility.hpp"

#include <vector>
#include <array>
#include <unordered_map>
#include <string>

enum class PaletteType
{
	UNKNOWN,
	BLOCKID,
	NAMESPACEID
};

enum class BlockOrder
{
	YZX, // Java
	XZY, // Bedrock, Alpha, Beta
};

// A compressed structure for a tile
// Should be 4 or 8 bytes depending on architecture
// Non-aligned it should be 3 bytes
struct TileData
{
	uint16_t index;
	uint8_t blockLight : 4;
	uint8_t skyLight : 4;
};

// A section storing all tiles for that section
// It works along with allocate on write, and can be reused
class SectionData
{
public:
	void setY(int32_t y);
	void setBlockOrder(BlockOrder order) { blockOrder = order; }

	void setBlocks(const std::vector<uint16_t> & blocks);
	void setBlockLight(const std::vector<int8_t> & blockLight);
	void updateBlockLight(const std::array<uint8_t, 4096> & blockLight);
	void setSkyLight(const std::vector<int8_t> & skyLight);

	void transform(const std::function<uint16_t(uint16_t)> & c);

	int32_t getY() const { return y; }
	const TileData & getTile(const utility::BlockPosition & pos) const;
	bool allocated() const;

	void clear();

private:
	std::vector<TileData> data;
	BlockOrder blockOrder = BlockOrder::YZX;
	int32_t y = 0;

	void allocate();
};

// Chunk storing all sections
// Also contains an heightmap for optimization
class Chunk
{
	typedef std::unordered_map<int8_t, SectionData> SectionDataList;
public:
	Chunk();

	void setX(int32_t x) { xPos = x; }
	void setZ(int32_t z) { zPos = z; }
	void setY(int32_t y) { yPos = y; }
	void setDataVersion(int32_t dataVersion);
	void setSection(const SectionData & section);
	void setSection(SectionData && section);
	void updateSection(const SectionData & section);
	void updateSection(SectionData && section);
	void setHeightMap(const std::vector<int32_t> & heightMap);
	void setHeightMap(std::vector<int32_t> && heightMap);
	void setPaletteType(PaletteType type);
	void addPalette(uint16_t id);
	void addPalette(const std::string & id);

	bool isValid() const;
	bool hasSection(const utility::BlockPosition & pos) const;
	const TileData & getTile(const utility::BlockPosition & pos) const;
	const SectionData & getSection(const utility::BlockPosition & pos) const;
	int32_t getHeight(const utility::PlanePosition & pos) const;
	int32_t getMinY() const { return minY; }
	int32_t getMaxY() const { return maxY; }
	int32_t getDataVersion() const;
	int32_t getX() const { return xPos; }
	int32_t getZ() const { return zPos; }
	int32_t getY() const { return yPos; }
	PaletteType getPaletteType() const;
	const std::vector<uint16_t> & getIDPalette() const;
	const std::vector<std::string> & getNSPalette() const;

	void merge(const Chunk & chunk);

private:
	SectionDataList data;
	struct {
		std::vector<std::string> ns;
		std::vector<uint16_t> id;
	} palette;
	PaletteType paletteType = PaletteType::UNKNOWN;
	std::vector<int32_t> heightMap;
	int32_t dataVersion = 0;
	int32_t xPos = 0, zPos = 0, yPos = 0, maxY, minY;

	void updateYMinMax(int32_t y);
};

#endif // CHUNK_HPP
