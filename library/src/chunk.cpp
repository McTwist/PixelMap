#include "chunk.hpp"

#include "util/nibble.hpp" // Move outside
#include "anvil/limits.hpp"

#include <unordered_map>
#include <algorithm>


static const TileData emptyTile{0, 0, 0};
static const SectionData emptySection{};

template<typename T>
void transform_chunk(const std::vector<std::reference_wrapper<SectionData>> & data, std::vector<T> & palette_to, const std::vector<T> & palette_from);

void SectionData::setY(int32_t _y)
{
	y = _y;
}

void SectionData::setBlocks(const std::vector<uint16_t> & d)
{
	allocate();
	if (d.size() == 1)
	{
		for (auto i = 0U; i < data.size(); ++i)
			data[i].index = d[0];
	}
	else
	{
		for (auto i = 0U; i < d.size(); ++i)
			data[i].index = d[i];
	}
}
void SectionData::setBlockLight(const std::vector<int8_t> &d)
{
	allocate();
	auto s = d.size() << 1;
	for (auto i = 0U; i < s; ++i)
		data[i].blockLight = static_cast<uint8_t>(nibble4(d, i));
}
void SectionData::updateBlockLight(const std::array<uint8_t, SECTION_SIZE> &d)
{
	allocate();
	for (auto i = 0U; i < d.size(); ++i)
		data[i].blockLight = d[i];
}
void SectionData::setSkyLight(const std::vector<int8_t> &d)
{
	allocate();
	auto s = d.size() << 1;
	for (auto i = 0U; i < s; ++i)
		data[i].skyLight = static_cast<uint8_t>(nibble4(d, i));
}

void SectionData::transform(const std::function<uint16_t(uint16_t)> & c)
{
	std::for_each(data.begin(), data.end(), [&c](TileData & t) { t.index = c(t.index); });
}

// Get tile from section
const TileData & SectionData::getTile(const utility::BlockPosition & pos) const
{
	if (blockOrder == BO_YZX)
		return data.at(std::size_t(
			pos.y * SECTION_X * SECTION_Z +
			pos.z * SECTION_X +
			pos.x
		));
	else
		return data.at(std::size_t(
			pos.x * SECTION_Y * SECTION_Z +
			pos.z * SECTION_Y +
			pos.y
		));
}

// Check if section have been allocated
bool SectionData::allocated() const
{
	return !data.empty();
}

// Clear the section
void SectionData::clear()
{
	// Only add needs to be cleared, as that value is optional
	for (auto & d : data)
	{
		d.index = 0;
		d.blockLight = 0;
		d.skyLight = 0;
	}
}

// Dynamically allocate if not allocated
inline void SectionData::allocate()
{
	if (data.size() == SECTION_SIZE)
		return;
	data.resize(SECTION_SIZE);
}

Chunk::Chunk()
{
	heightMap.resize(SECTION_AREA);
	maxY = std::numeric_limits<decltype(maxY)>::min();
	minY = std::numeric_limits<decltype(minY)>::max();
}

void Chunk::setDataVersion(int32_t _dataVersion)
{
	dataVersion = _dataVersion;
}

void Chunk::setSection(const SectionData & section)
{
	auto y = section.getY();
	updateYMinMax(y);
	data.emplace(std::make_pair(y, section));
}

void Chunk::setSection(SectionData && section)
{
	auto y = section.getY();
	updateYMinMax(y);
	data.emplace(std::make_pair(y, std::move(section)));
}

void Chunk::updateSection(const SectionData & section)
{
	auto y = section.getY();
	updateYMinMax(y);
	data.insert_or_assign(y, section);
}

void Chunk::updateSection(SectionData && section)
{
	auto y = section.getY();
	updateYMinMax(y);
	data.insert_or_assign(y, std::move(section));
}

void Chunk::setHeightMap(const std::vector<int32_t> & d)
{
	std::copy(d.begin(), d.end(), heightMap.begin());
}

void Chunk::setHeightMap(std::vector<int32_t> && d)
{
	heightMap = std::move(d);
}

void Chunk::setPaletteType(PaletteType type)
{
	paletteType = type;
}

void Chunk::addPalette(uint16_t id)
{
	assert(paletteType == PT_BLOCKID);
	palette.id.push_back(id);
}

void Chunk::addPalette(const std::string & id)
{
	assert(paletteType == PT_NAMESPACEID);
	palette.ns.push_back(id);
}

bool Chunk::isValid() const
{
	return paletteType != PT_UNKNOWN && palette.id.empty() != palette.ns.empty();
}

bool Chunk::hasSection(const utility::BlockPosition & pos) const
{
	auto i = (pos.y - getMinY()) / SECTION_Y + (getMinY() / SECTION_Y);
	auto it = data.find(i);
	if (it == data.end())
		return false;
	return it->second.allocated();
}

const TileData & Chunk::getTile(const utility::BlockPosition & pos) const
{
	auto i = (pos.y - getMinY()) / SECTION_Y + (getMinY() / SECTION_Y);
	auto it = data.find(i);
	if (it == data.end())
		return emptyTile;
	return it->second.getTile({
		(pos.x % SECTION_X + SECTION_X) % SECTION_X,
		(pos.y % SECTION_Y + SECTION_Y) % SECTION_Y,
		(pos.z % SECTION_Z + SECTION_Z) % SECTION_Z
	});
}

const SectionData & Chunk::getSection(const utility::BlockPosition & pos) const
{
	auto i = (pos.y - getMinY()) / SECTION_Y + (getMinY() / SECTION_Y);
	auto it = data.find(i);
	if (it == data.end())
		return emptySection;
	return it->second;
}

int32_t Chunk::getHeight(const utility::PlanePosition & pos) const
{
	if (heightMap.empty()) return 0;
	return heightMap[
		((pos.y % SECTION_Z + SECTION_Z) % SECTION_Z) * SECTION_X +
		((pos.x % SECTION_X + SECTION_X) % SECTION_X)
	];
}

int32_t Chunk::getDataVersion() const
{
	return dataVersion;
}

PaletteType Chunk::getPaletteType() const
{
	return paletteType;
}

const std::vector<uint16_t> & Chunk::getIDPalette() const
{
	return palette.id;
}

const std::vector<std::string> & Chunk::getNSPalette() const
{
	return palette.ns;
}

void Chunk::merge(const Chunk & chunk)
{
	// Only copy what needs to be copied
	if (getPaletteType() == chunk.getPaletteType())
	{
		std::vector<std::reference_wrapper<SectionData>> transpose;
		for (auto & section : data)
			if (chunk.data.find(section.first) == chunk.data.end())
				transpose.emplace_back(section.second);
		for (auto & section : chunk.data)
			data.insert_or_assign(section.first, section.second);
		if (getPaletteType() == PT_BLOCKID)
			transform_chunk(transpose, palette.id, chunk.palette.id);
		else if (getPaletteType() == PT_NAMESPACEID)
			transform_chunk(transpose, palette.ns, chunk.palette.ns);
	}
	// Replace everything
	else
	{
		data = chunk.data;
		if (getPaletteType() == PT_BLOCKID)
			palette.id = chunk.palette.id;
		else if (getPaletteType() == PT_NAMESPACEID)
			palette.ns = chunk.palette.ns;
	}
	heightMap = chunk.heightMap;
	dataVersion = dataVersion;
}

inline void Chunk::updateYMinMax(int32_t y)
{
	auto _y = (y * SECTION_Y) + SECTION_Y - 1;
	if (_y > maxY)
		maxY = _y;
	if (y < minY)
		minY = y;
}

/*
 * Private functions
 */

template<typename T>
void transform_chunk(const std::vector<std::reference_wrapper<SectionData>> & data, std::vector<T> & palette_to, const std::vector<T> & palette_from)
{
	std::unordered_map<T, uint16_t> reversed_palette;
	for (std::size_t i = 0; i < palette_from.size(); ++i)
		reversed_palette[palette_from[i]] = i;
	// Add new ids
	for (auto id : palette_to)
		if (reversed_palette.find(id) == reversed_palette.end())
			reversed_palette[id] = reversed_palette.size();
	for (auto section : data)
	{
		section.get().transform([&palette_to, &reversed_palette](uint16_t t) -> uint16_t {
			return reversed_palette[palette_to[t]];
		});
	}
	palette_to.clear();
	palette_to.resize(reversed_palette.size());
	for (auto it : reversed_palette)
		palette_to[it.second] = it.first;
}
