#include "format/region.hpp"

#include "platform.hpp"
#include "string.hpp"
#include "util/endianess.hpp"
#include "semaphore.hpp"

#include <sstream>
#include <iostream>
#include <filesystem>
#include <regex>

// The size of a chunk
constexpr uint32_t CHUNK_SIZE = 4096;
constexpr uint32_t HEADER_CHUNKS = 2;
constexpr uint32_t HEADER_SIZE = CHUNK_SIZE * HEADER_CHUNKS;

/*
 * Internal functionality
 */
uint32_t getHeader(int x, int z);
uint32_t getIndex(int x, int z);

namespace region
{

const std::string_view ChunkData::getCustomFormat() const
{
	if (compression_type != COMPRESSION_CUSTOM)
		return {};
	auto len = endianess::fromBig<uint16_t>(data.data());
	const char * p = reinterpret_cast<const char *>(data.data());
	return {p+2, len};
}

Region::Region(const std::string& path, RegionType type) noexcept :
	path(path), type(type)
{
}

int Region::getChunkTimestamp(int x, int z)
{
	auto pos = std::make_pair(x >> 5, z >> 5);
	auto it = regions.find(pos);
	if (it == regions.end())
	{
		regions.insert(std::make_pair(pos, std::make_shared<RegionFile>(pos.first, pos.second, type)));
		it = regions.find(pos);
	}
	it->second->open(path);
	auto timestamp = it->second->getChunkTimestamp(x & 31, z & 31);
	it->second->close();
	return timestamp;
}

std::shared_ptr<ChunkData> Region::getChunk(int x, int z)
{
	auto pos = std::make_pair(x >> 5, z >> 5);
	auto it = regions.find(pos);
	if (it == regions.end())
	{
		regions.insert(std::make_pair(pos, std::make_shared<RegionFile>(pos.first, pos.second, type)));
		it = regions.find(pos);
	}
	it->second->open(path);
	auto chunk = it->second->getChunk(x & 31, z & 31);
	it->second->close();
	return chunk;
}

Region::iterator Region::begin()
{
	// Get all files
	populateFromPath();

	return iterator(regions.begin(), *this);
}

Region::iterator Region::end()
{
	return iterator(regions.end(), *this);
}

Region::chunk_iterator Region::beginChunk()
{
	return chunk_iterator(begin(), end());
}

Region::chunk_iterator Region::endChunk()
{
	return chunk_iterator(end(), end());
}

void Region::populateFromPath()
{
	if (!std::filesystem::is_directory(path))
		return;

	regions.clear();

	std::regex r;
	if (type == RegionType::ANVIL)
		r = "^r\\.(-?[0-9]+)\\.(-?[0-9]+)\\.mca$";
	else if (type == RegionType::BETA)
		r = "^r\\.(-?[0-9]+)\\.(-?[0-9]+)\\.mcr$";
	std::error_code ec;
	for (const auto & entry : std::filesystem::directory_iterator{path, ec})
	{
		// Not interested in directories
		if (entry.is_directory())
			continue;

		auto name = entry.path().filename().string();
		std::smatch m;
		if (!std::regex_match(name, m, r))
			continue;
		if (m.size() != 3)
			continue;

		int x, z;

		try
		{
			x = std::stoi(m[1].str());
			z = std::stoi(m[2].str());
		}
		catch(...)
		{
			continue;
		}

		auto pos = std::make_pair(x, z);
		regions.insert(std::make_pair(pos, std::make_shared<RegionFile>(pos.first, pos.second, type)));
	}
}



RegionFile::RegionFile(int x, int z,  RegionType type) noexcept :
	rx(x), rz(z), type(type)
{
	headers.resize(CHUNK_SIZE >> 2);
}

bool RegionFile::open(const std::string & _path)
{
	path = _path;
	return openFile(platform::path::join(path, file()));
}

bool RegionFile::openFile(const std::string & file)
{
	if (!SharedFile::openFile(file))
		return false;

	return loadHeader();
}

std::string RegionFile::file() const
{
	return string::format("r.", rx, ".", rz, (type == RegionType::ANVIL ? ".mca" : ".mcr"));
}

void RegionFile::clear()
{
	cache.clear();
	decltype(cache)().swap(cache);
}

std::shared_ptr<ChunkData> RegionFile::getChunk(int x, int z)
{
	return getChunk(headers[getIndex(x, z)]);
}

bool RegionFile::containsChunk(int x, int z) const
{
	return headers[getIndex(x, z)].offset >= 2;
}

RegionFile::iterator RegionFile::begin()
{
	return iterator(headers.begin(), this);
}

RegionFile::iterator RegionFile::end()
{
	return iterator(headers.end(), this);
}

int RegionFile::getChunkTimestamp(int x, int z)
{
	return headers[getIndex(x, z)].timestamp;
}

bool RegionFile::loadHeader()
{
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
		headers[i].timestamp = endianess::fromBig<int32_t>(&buffer[j]);
	}

	return false;
}

std::shared_ptr<ChunkData> RegionFile::getChunk(const Header & header)
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
	if (offset + 4 > cache.size())
	{
		setError("Offset outside of file");
		return chunk;
	}
	auto ptr = const_cast<const uint8_t *>(cache.data()) + offset;
	auto length = endianess::fromBig<uint32_t>(ptr);
	if (offset + length > cache.size())
	{
		setError("Chunk outside of file");
		return chunk;
	}
	if (length + 4 > CHUNK_SIZE * header.sector_count)
	{
		setError("Chunk outside of sector");
		return chunk;
	}
	auto compression_type = static_cast<ChunkData::CompressionType>(*(ptr + 4));

	auto cx = (rx * 32) + int32_t(header.i & 31);
	auto cz = (rz * 32) + int32_t(header.i >> 5);

	/*
	 * Note: Each chunk can be at most 1MiB, so this format was introduced
	 * to allow no limit on a chunk size. This is very rare and was added
	 * specifically because a single person had encountered this limitation.
	 */
	if (compression_type & 0x80)
	{
		auto external_chunk = std::make_shared<RegionChunk>(cx, cz, static_cast<ChunkData::CompressionType>(compression_type - 128));
		if (!external_chunk->open(path))
			return chunk;
		chunk = external_chunk->getChunk();
		external_chunks.emplace_back(external_chunk);
	}
	else
	{
		chunk = std::make_shared<ChunkData>(cx, cz);
		chunk->compression_type = compression_type;
		chunk->data = VectorData{ptr + 5, length - 1};
	}

	return chunk;
}

inline void RegionFile::preloadCache()
{
	if (!cache.empty())
		return;
	auto _size = size();
	_size -= HEADER_SIZE;
	seek(HEADER_SIZE);
	cache.resize(_size);
	read(cache.data(), _size);
}



RegionChunk::RegionChunk(int x, int z, ChunkData::CompressionType _compression) noexcept :
	cx(x), cz(z), compression(_compression)
{}

bool RegionChunk::open(const std::string & path)
{
	return openFile(platform::path::join(path, file()));
}

bool RegionChunk::openFile(const std::string & file)
{
	if (!SharedFile::openFile(file))
		return false;

	data = readAll();
	return true;
}

std::string RegionChunk::file() const
{
	return string::format("c.", cx, ".", cz, ".mcc");
}

std::shared_ptr<ChunkData> RegionChunk::getChunk()
{
	std::shared_ptr<ChunkData> chunk;
	if (!isOpen())
		return chunk;
	if (data.empty())
		return chunk;
	
	chunk = std::make_shared<ChunkData>(cx, cz);
	chunk->compression_type = compression;
	chunk->data = VectorData{data.data(), data.size()};
	return chunk;
}

/*
 * Iterators
 */

RegionFile::iterator::iterator(Headers::iterator _it, RegionFile * _region) :
	it(_it),
	region(_region)
{
	ensureValidIterator();
}
RegionFile::iterator& RegionFile::iterator::operator++()
{
	++it;
	ensureValidIterator();
	return *this;
}
RegionFile::iterator RegionFile::iterator::operator++(int)
{
	iterator tmp(*this);
	operator++();
	return tmp;
}
bool RegionFile::iterator::operator==(const iterator & rhs) const
{
	return it == rhs.it;
}
bool RegionFile::iterator::operator!=(const iterator & rhs) const
{
	return it != rhs.it;
}
RegionFile::iterator::value_type RegionFile::iterator::operator*()
{
	return region->getChunk(*it);
}
RegionFile::iterator::pointer RegionFile::iterator::operator->()
{
	// Go around pointer issue
	return operator*().get();
}

// Make sure that the iterator is valid
void RegionFile::iterator::ensureValidIterator()
{
	// Avoid having an invalid one
	while (it != region->headers.end() && it->offset < 2)
		++it;
}


Region::iterator::iterator(RegionsMap::iterator _it, Region & _anvil) :
	it(_it),
	anvil(_anvil)
{
	if (it == anvil.regions.end())
		return;
	it->second->open(anvil.path);
}
Region::iterator & Region::iterator::operator++()
{
	if (it->second.use_count() == 1)
		it->second->close();
	++it;
	if (it != anvil.regions.end())
		it->second->open(anvil.path);
	return *this;
}
Region::iterator Region::iterator::operator++(int)
{
	iterator tmp(*this);
	operator++();
	return tmp;
}
bool Region::iterator::operator==(const iterator & rhs) const
{
	return it == rhs.it;
}
bool Region::iterator::operator!=(const iterator & rhs) const
{
	return it != rhs.it;
}
Region::iterator::reference Region::iterator::operator*()
{
	return it->second;
}
Region::iterator::pointer Region::iterator::operator->()
{
	// Go around pointer issue
	return it->second.get();
}


Region::chunk_iterator::chunk_iterator(Region::iterator reg, Region::iterator end) :
	region_it(reg),
	region_end(end)
{
	if (region_it == region_end)
		return;
	chunk_it = RegionFile::iterator((*region_it)->begin());
	ensureValidIterator();
}
Region::chunk_iterator& Region::chunk_iterator::operator++()
{
	++chunk_it;
	ensureValidIterator();
	return *this;
}
Region::chunk_iterator Region::chunk_iterator::operator++(int)
{
	chunk_iterator tmp(*this);
	operator++();
	return tmp;
}
bool Region::chunk_iterator::operator==(const chunk_iterator & rhs) const
{
	return region_it == rhs.region_it;
}
bool Region::chunk_iterator::operator!=(const chunk_iterator & rhs) const
{
	return region_it != rhs.region_it;
}
Region::chunk_iterator::value_type Region::chunk_iterator::operator*()
{
	return *chunk_it;
}
Region::chunk_iterator::pointer Region::chunk_iterator::operator->()
{
	// Go around pointer issue
	return chunk_it.operator->();
}
// Make sure that the iterator is valid
void Region::chunk_iterator::ensureValidIterator()
{
	while (chunk_it == (*region_it)->end())
	{
		++region_it;
		if (region_it == region_end)
			return;
		chunk_it = (*region_it)->begin();
	}
}

} // namespace region

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
