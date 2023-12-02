#include "format/anvil.hpp"

#include "platform.hpp"
#include "string.hpp"
#include "util/endianess.hpp"
#include "semaphore.hpp"

#include <sstream>
#include <iostream>
#include <filesystem>

// The size of a chunk
constexpr uint32_t CHUNK_SIZE = 4096;
constexpr uint32_t HEADER_CHUNKS = 2;
constexpr uint32_t HEADER_SIZE = CHUNK_SIZE * HEADER_CHUNKS;

/*
 * Internal functionality
 */
uint32_t getHeader(int x, int z);
uint32_t getIndex(int x, int z);

namespace anvil
{

Anvil::Anvil(const std::string& path) noexcept :
	path(path)
{
}

int Anvil::getChunkTimestamp(int x, int z)
{
	auto pos = std::make_pair(x >> 5, z >> 5);
	auto it = regions.find(pos);
	if (it == regions.end())
	{
		regions.insert(std::make_pair(pos, std::make_shared<AnvilRegion>(pos.first, pos.second)));
		it = regions.find(pos);
	}
	it->second->open(path);
	auto timestamp = it->second->getChunkTimestamp(x & 31, z & 31);
	it->second->close();
	return timestamp;
}

std::shared_ptr<ChunkData> Anvil::getChunk(int x, int z)
{
	auto pos = std::make_pair(x >> 5, z >> 5);
	auto it = regions.find(pos);
	if (it == regions.end())
	{
		regions.insert(std::make_pair(pos, std::make_shared<AnvilRegion>(pos.first, pos.second)));
		it = regions.find(pos);
	}
	it->second->open(path);
	auto chunk = it->second->getChunk(x & 31, z & 31);
	it->second->close();
	return chunk;
}

Anvil::iterator Anvil::begin()
{
	// Get all files
	populateFromPath();

	return iterator(regions.begin(), *this);
}

Anvil::iterator Anvil::end()
{
	return iterator(regions.end(), *this);
}

Anvil::chunk_iterator Anvil::beginChunk()
{
	return chunk_iterator(begin(), end());
}

Anvil::chunk_iterator Anvil::endChunk()
{
	return chunk_iterator(end(), end());
}

bool Anvil::validateFileName(const std::string & file)
{
	auto len = long(file.length()) - 4;
	// Too short name
	if (len < 0)
		return false;
	// Verify integrity
	if (file[0] != 'r' || file.substr(std::size_t(len)) != ".mca")
		return false;
	// Correct format
	auto first = file.find(".");
	if (first == std::string::npos)
		return false;
	auto second = file.find(".", first + 1);
	if (second == std::string::npos)
		return false;
	auto third = file.find(".", second + 1);
	if (third == std::string::npos)
		return false;
	return true;
}

void Anvil::populateFromPath()
{
	if (!std::filesystem::is_directory(path))
		return;

	regions.clear();

	std::error_code ec;
	for (const auto & entry : std::filesystem::directory_iterator{path, ec})
	{
		// Not interested in directories
		if (entry.is_directory())
			continue;

		auto name = entry.path().filename().string();
		// Too short name
		if (name.length() < 4)
			continue;
		// Verify integrity
		if (name[0] != 'r' && name.substr(name.length() - 4) != ".mca")
			continue;
		// Correct format
		auto first = name.find(".");
		if (first == std::string::npos)
			continue;
		auto second = name.find(".", first + 1);
		if (second == std::string::npos)
			continue;
		auto third = name.find(".", second + 1);
		if (third == std::string::npos)
			continue;
		// Nullify them
		name[second] = 0;
		name[third] = 0;

		int x, z;

		try
		{
			x = std::stoi(&name[first + 1]);
			z = std::stoi(&name[second + 1]);
		}
		catch (...)
		{
			continue;
		}

		auto pos = std::make_pair(x, z);
		regions.insert(std::make_pair(pos, std::make_shared<AnvilRegion>(pos.first, pos.second)));
	}
}



AnvilRegion::AnvilRegion(int x, int z) noexcept :
	rx(x), rz(z)
{
	headers.resize(CHUNK_SIZE >> 2);
}

bool AnvilRegion::open(const std::string & path)
{
	return openFile(platform::path::join(path, file()));
}

bool AnvilRegion::openFile(const std::string & file)
{
	if (!SharedFile::openFile(file))
		return false;

	uint8_t buffer[CHUNK_SIZE];

	if (!read(buffer, sizeof(buffer)))
		return false;

	Header header;

	amount_chunks = 0;

	for (uint32_t i = 0, j = 0; j < sizeof(buffer); ++i, j+= 4)
	{
		header.i = i;
		header.offset = endianess::fromBig<uint32_t>(&buffer[j], 3);
		if (header.offset >= 2)
			++amount_chunks;
		header.sector_count = buffer[j + 3];
		headers[i] = header;
	}

	if (!read(buffer, sizeof(buffer)))
		return false;

	for (uint32_t i = 0, j = 0; j < sizeof(buffer); ++i, j+= 4)
	{
		headers[i].timestamp = endianess::fromBig<int>(&buffer[j]);
	}

	return true;
}

std::string AnvilRegion::file() const
{
	return string::format("r.", rx, ".", rz, ".mca");
}

std::shared_ptr<ChunkData> AnvilRegion::getChunk(int x, int z)
{
	return getChunk(headers[getIndex(x, z)]);
}

bool AnvilRegion::containsChunk(int x, int z) const
{
	return headers[getIndex(x, z)].offset >= 2;
}

AnvilRegion::iterator AnvilRegion::begin()
{
	return iterator(headers.begin(), this);
}

AnvilRegion::iterator AnvilRegion::end()
{
	return iterator(headers.end(), this);
}

int AnvilRegion::getChunkTimestamp(int x, int z)
{
	return headers[getIndex(x, z)].timestamp;
}

std::shared_ptr<ChunkData> AnvilRegion::getChunk(const Header & header)
{
	std::shared_ptr<ChunkData> chunk;
	if (!isOpen())
		return chunk;
	if (header.offset < HEADER_CHUNKS)
		return chunk;
	preloadCache();
	/*
	 * Note: This supports up to 16 TB file, while 4GB would be sufficient.
	 * All chunks together could reach a theoretical maximum of 4TB, though.
	 */
	auto offset = uint64_t(header.offset - HEADER_CHUNKS) * CHUNK_SIZE;
	if (offset > cache.size())
		return chunk;
	auto ptr = const_cast<const uint8_t *>(cache.data()) + offset;
	auto length = endianess::fromBig<uint32_t>(ptr);
	if (offset + length > cache.size())
		return chunk;
	auto compression_type = static_cast<ChunkData::CompressionType>(*(ptr + 4));

	chunk = std::make_shared<ChunkData>(
		(rx * 32) + int32_t(header.i & 31),
		(rz * 32) + int32_t(header.i >> 5));
	chunk->compression_type = compression_type;
	chunk->data = VectorData{ptr + 5, length - 1};

	return chunk;
}

inline void AnvilRegion::preloadCache()
{
	if (!cache.empty())
		return;
	auto _size = size();
	_size -= HEADER_SIZE;
	seek(HEADER_SIZE);
	cache.resize(_size);
	read(cache.data(), _size);
}

/*
 * Iterators
 */

AnvilRegion::iterator::iterator(Headers::iterator _it, AnvilRegion * _region) :
	it(_it),
	region(_region)
{
	ensureValidIterator();
}
AnvilRegion::iterator& AnvilRegion::iterator::operator++()
{
	++it;
	ensureValidIterator();
	return *this;
}
AnvilRegion::iterator AnvilRegion::iterator::operator++(int)
{
	iterator tmp(*this);
	operator++();
	return tmp;
}
bool AnvilRegion::iterator::operator==(const iterator & rhs) const
{
	return it == rhs.it;
}
bool AnvilRegion::iterator::operator!=(const iterator & rhs) const
{
	return it != rhs.it;
}
AnvilRegion::iterator::value_type AnvilRegion::iterator::operator*()
{
	return region->getChunk(*it);
}
ChunkData * AnvilRegion::iterator::operator->()
{
	// Go around pointer issue
	return operator*().get();
}

// Make sure that the iterator is valid
void AnvilRegion::iterator::ensureValidIterator()
{
	// Avoid having an invalid one
	while (it != region->headers.end() && it->offset < 2)
		++it;
}


Anvil::iterator::iterator(RegionsMap::iterator _it, Anvil & _anvil) :
	it(_it),
	anvil(_anvil)
{
	if (it == anvil.regions.end())
		return;
	it->second->open(anvil.path);
}
Anvil::iterator & Anvil::iterator::operator++()
{
	if (it->second.use_count() == 1)
		it->second->close();
	++it;
	if (it != anvil.regions.end())
		it->second->open(anvil.path);
	return *this;
}
Anvil::iterator Anvil::iterator::operator++(int)
{
	iterator tmp(*this);
	operator++();
	return tmp;
}
bool Anvil::iterator::operator==(const iterator & rhs) const
{
	return it == rhs.it;
}
bool Anvil::iterator::operator!=(const iterator & rhs) const
{
	return it != rhs.it;
}
Anvil::iterator::reference Anvil::iterator::operator*()
{
	return it->second;
}
AnvilRegion * Anvil::iterator::operator->()
{
	// Go around pointer issue
	return it->second.get();
}


Anvil::chunk_iterator::chunk_iterator(Anvil::iterator reg, Anvil::iterator end) :
	region_it(reg),
	region_end(end)
{
	if (region_it == region_end)
		return;
	chunk_it = (*region_it)->begin();
	ensureValidIterator();
}
Anvil::chunk_iterator& Anvil::chunk_iterator::operator++()
{
	++chunk_it;
	ensureValidIterator();
	return *this;
}
Anvil::chunk_iterator Anvil::chunk_iterator::operator++(int)
{
	chunk_iterator tmp(*this);
	operator++();
	return tmp;
}
bool Anvil::chunk_iterator::operator==(const chunk_iterator & rhs) const
{
	return region_it == rhs.region_it;
}
bool Anvil::chunk_iterator::operator!=(const chunk_iterator & rhs) const
{
	return region_it != rhs.region_it;
}
Anvil::chunk_iterator::value_type Anvil::chunk_iterator::operator*()
{
	return *chunk_it;
}
ChunkData * Anvil::chunk_iterator::operator->()
{
	// Go around pointer issue
	return chunk_it.operator->();
}
// Make sure that the iterator is valid
void Anvil::chunk_iterator::ensureValidIterator()
{
	while (chunk_it == (*region_it)->end())
	{
		++region_it;
		if (region_it == region_end)
			return;
		chunk_it = (*region_it)->begin();
	}
}

} // namespace anvil

/*
 * Internal functionality
 */
inline uint32_t getHeader(int x, int z)
{
	return 4 * getIndex(x, z);
}

inline uint32_t getIndex(int x, int z)
{
	return uint32_t((x & 31) + ((z & 31) << 5));
}
