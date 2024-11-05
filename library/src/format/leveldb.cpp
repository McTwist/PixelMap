#include "format/leveldb.hpp"

#include "platform.hpp"
#include "util/endianess.hpp"
#include "vectorview.hpp"
#include "util/nibble.hpp"
#include "util/compression.hpp"
#include "format/nbt.hpp"
#include "format/varint.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h> // fmt

#include <sstream>
#include <algorithm>
#include <cmath>
#include <filesystem>
#include <regex>

/*
Endian: little
LevelDB format
block {
	block_data: uint8[n] {
		entry: [] {
			shared_bytes: varint32
			unshared_bytes: varint32
			value_bytes: varint32
			key_delta: char[unshared_bytes]
			value: char[value_length]
		}
		restarts: uint32[num_restarts]
		num_restarts: uint32
	}
	type: uint8
	crc: uint32
}
block_handle: {
	offset: uint64
	size: uint64
}

data_blocks: block[](nbt)
meta_blocks: block[]
metaindex_block: block(block_handle)
index_block: block(block_handle)
footer: {
	metaindex_handle: char[p]
	index_handle: char[q]
static void skip_chunk_key();
	padding: char[40-p-q]
	magic: uint64 // 0xdb4775248b80fb57 
}

Searching:
footer -(off)> index(compr) -(off)> block(compr) -(iter)> key -(next)> chunk sub sections
*/

/*
Log format
block: <=32KiB {
	record: uint8[] {
		checksum: uint32
		length: uint16
		type: uint8
		data: uint8[length] {
			sequence: varint64
			count: varint32
			values {
				tag: uint8
				key_len: varint32
				key: char[key_len]
				if tag == 1:
					val_len: varint32
					val: char[val_len]
			}
		}
	}
}
*/

constexpr uint64_t FOOTER_SIZE = 48;
constexpr uint64_t MAGIC = 0xdb4775248b80fb57ULL;
enum CompressorType : uint8_t
{
	COMPRESSOR_RAW = 0,
	COMPRESSOR_SNAPPY = 1,
	COMPRESSOR_ZLIB = 2,
	COMPRESSOR_ZSTD = 3,
	COMPRESSOR_ZLIBRAW = 4,
};
constexpr uint64_t BLOCK_SIZE = 32768;
constexpr uint64_t HEADER_SIZE = 7;
enum RecordType : uint8_t
{
	TYPE_ZERO = 0,
	TYPE_FULL = 1,
	TYPE_FIRST = 2,
	TYPE_MIDDLE = 3,
	TYPE_LAST = 4,
};
constexpr uint32_t BUFFER_SIZE = 4096;

static std::tuple<uint64_t, uint64_t> read_block_handle(const uint8_t *&);
static void skip_block_handle(const uint8_t *&);

static const uint8_t * get_block_end_pos(const std::vector<uint8_t> & block);

//static std::vector<uint8_t> load_block(const std::vector<uint8_t> & data, uint64_t offset, uint64_t size);
static std::vector<uint8_t> load_block(LevelDB::VectorData data, uint64_t offset, uint64_t size);

static std::vector<uint8_t> load_block_type(uint8_t type, const std::vector<uint8_t> & data);
static std::vector<uint8_t> load_block_type(uint8_t type, LevelDB::VectorData data);

enum ValueType : uint8_t
{
	TYPE_Data3D = 43,
	TYPE_Version = 44,
	TYPE_Data2D = 45, // < 1.18.0
	TYPE_Data2DLegacy = 46, // < 1.0.0
	TYPE_SubChunkPrefix = 47,
	TYPE_LegacyTerrain = 48, // < 1.0.0
	TYPE_BlockEntity = 49,
	TYPE_Entity = 50,
	TYPE_PendingTicks = 51,
	TYPE_LegacyBlockExtraData = 52, // < 1.2.13
	TYPE_BiomeState = 53,
	TYPE_FinalizedState = 54,
	TYPE_BorderBlocks = 56, // Education Edition
	TYPE_HardcodedSpawners = 57,
	TYPE_RandomTicks = 58,
	TYPE_Checksums = 59, // < 1.18.0
	TYPE_LegacyVersion = 118, // < 1.16.100
};
struct ChunkKey {
	int32_t x, z;
	int32_t dimension = 0; // Optional
	uint8_t type;
	int8_t index = -1; // Optional
};
static ChunkKey read_chunk_key(const std::vector<uint8_t> & key);

class BlockParser
{
public:
	BlockParser() = default;
	BlockParser(const BlockParser &) = default;
	explicit BlockParser(const uint8_t * begin, const uint8_t * end);
	std::pair<std::vector<uint8_t>, LevelDB::VectorData> next();
	bool has() const;
private:
	const uint8_t * ptr = nullptr, * end = nullptr;
	std::vector<uint8_t> prev_key;
};

namespace LevelDB
{

LevelDB::LevelDB(const std::string & path) noexcept
	: path(path)
{
	populateFromPath();
}

void LevelDB::populateFromPath()
{
	if (!std::filesystem::is_directory(path))
		return;

	levels.clear();
	log.clear();

	std::regex r_ldb(".+\\.ldb"), r_log(".+\\.log");
	std::error_code ec;
	for (const auto & entry : std::filesystem::directory_iterator{path, ec})
	{
		// Not interested in directories
		if (entry.is_directory())
			continue;

		auto name = entry.path().filename().string();
		if (std::regex_match(name, r_ldb))
			levels.emplace_back(std::make_shared<LevelFile>(name));
		else if (std::regex_match(name, r_log))
			log = name;
	}

	// Note: Oldest to newest to process replacement
	std::sort(levels.begin(), levels.end(), [] (std::shared_ptr<LevelFile> & a, std::shared_ptr<LevelFile> & b) {
		return a->file() < b->file();
	});
}

LevelDB::iterator LevelDB::begin()
{
	return iterator(levels.begin(), *this);
}

LevelDB::iterator LevelDB::end()
{
	return iterator(levels.end(), *this);
}

std::size_t LevelDB::size() const
{
	return levels.size();
}

std::shared_ptr<LevelFile> LevelDB::getLog() const
{
	auto file = std::make_shared<LevelFile>(log);
	file->open(path);
	return file;
}


LevelFile::LevelFile(const std::string & file) noexcept
	: _file(file)
{
}

bool LevelFile::open(const std::string & path)
{
	return openFile(platform::path::join(path, _file));
}

/*
 * Iterators
 */
LevelDB::iterator::iterator(Levels::iterator _it, LevelDB & _leveldb) :
	it(_it),
	leveldb(_leveldb)
{
	if (it == leveldb.levels.end())
		return;
	(*it)->open(leveldb.path);
}
LevelDB::iterator & LevelDB::iterator::operator++()
{
	if ((*it).unique())
		(*it)->close();
	++it;
	if (it != leveldb.levels.end())
		(*it)->open(leveldb.path);
	return *this;
}
LevelDB::iterator LevelDB::iterator::operator++(int)
{
	iterator tmp(*this);
	operator++();
	return tmp;
}
bool LevelDB::iterator::operator==(const iterator & rhs) const
{
	return it == rhs.it;
}
bool LevelDB::iterator::operator!=(const iterator & rhs) const
{
	return it != rhs.it;
}
LevelDB::iterator::reference LevelDB::iterator::operator*()
{
	return *it;
}
LevelDB::iterator::pointer LevelDB::iterator::operator->()
{
	// Go around pointer issue
	return it->get();
}


std::ptrdiff_t Reader::parse(std::vector<uint8_t> & data, Visitor & visitor)
{
	return parse({data.data(), data.size()}, visitor);
}
std::ptrdiff_t Reader::parse(VectorView<uint8_t> data, Visitor & visitor)
{
	return parse(data, [&visitor](const std::vector<uint8_t> & key, const VectorData & data)
	{
		return visitor.visit(key, data);
	});
}

std::ptrdiff_t Reader::parse(std::vector<uint8_t> & data, std::function<void(const std::vector<uint8_t> &, const VectorData &)> visit)
{
	return parse({data.data(), data.size()}, visit);
}

std::ptrdiff_t Reader::throwError(const std::string & err)
{
	error = err;
	return -1;
}

std::ptrdiff_t LevelReader::parse(VectorView<uint8_t> data, std::function<void(const std::vector<uint8_t> &, const VectorData &)> visit)
{
	std::unordered_map<uint64_t, uint64_t> block_indices;
	uint64_t offset, size, data_size = 0;
	{
		if (data.size() < FOOTER_SIZE)
			return throwError("Unable to read footer");
		auto magic = endianess::fromLittle<uint64_t, const uint8_t *>(data.data() + data.size() - sizeof(uint64_t));
		if (magic != MAGIC)
			return throwError("Invalid magic");
		const uint8_t * ptr = data.data() + data.size() - FOOTER_SIZE;
		skip_block_handle(ptr); // metaindex_handle
		std::tie(offset, size) = read_block_handle(ptr);
	}
	{
		const uint8_t * ptr = data.data() + offset;
		auto type = ptr[size];
		auto block = load_block_type(type, {ptr, size});
		auto it = BlockParser(block.data(), get_block_end_pos(block));
		while (it.has())
		{
			auto v = it.next().second.data();
			auto [boffset, bsize] = read_block_handle(v);
			data_size = std::max(data_size, boffset + bsize);
			block_indices.insert(std::make_pair(boffset, bsize));
		}
		data_size += 5; // type and crc
		if (data.size() < data_size)
			return throwError("Invalid data block");
		data = {data.data(), data_size};
	}

	BlockParser kit;
	std::vector<uint8_t> block;
	for (auto it : block_indices)
	{
		block = load_block(
			{data.data(), data.size()},
			it.first, it.second);
		if (block.empty())
			return throwError("Unable to read block");
		kit = BlockParser(block.data(), get_block_end_pos(block));
		while (kit.has())
		{
			auto next = kit.next();

			visit(next.first, next.second);
		}
	}
	return 0;
}


std::ptrdiff_t LogReader::parse(VectorView<uint8_t> data, std::function<void(const std::vector<uint8_t> &, const VectorData &)> visit)
{
	uint64_t it = 0;
	std::vector<std::vector<uint8_t>> blocks;
	std::vector<uint8_t> block;
	uint8_t prev_type = TYPE_ZERO;
	while (it + HEADER_SIZE < data.size())
	{
		if (it % BLOCK_SIZE > BLOCK_SIZE - HEADER_SIZE)
		{
			it = (it + HEADER_SIZE) % BLOCK_SIZE;
			continue; // Check size again
		}
		auto checksum = endianess::fromLittle<uint32_t>(data.data() + it);
		it += sizeof(checksum);
		auto length = endianess::fromLittle<uint16_t>(data.data() + it);
		it += sizeof(length);
		auto type = *(data.data() + it);
		it += sizeof(type);
		if (it + length > data.size()) // EOF
			break;
		switch (type)
		{
		case TYPE_FULL:
			if (!block.empty())
				return throwError("Full, in fragment");
			block.insert(block.end(), data.data() + it, data.data() + it + length);
			blocks.emplace_back(std::move(block));
			block.clear();
			break;
		case TYPE_FIRST:
			if (!block.empty())
				return throwError("First, in fragment");
			block.insert(block.end(), data.data() + it, data.data() + it + length);
			break;
		case TYPE_MIDDLE:
			if (block.empty() && prev_type != TYPE_FIRST)
				return throwError("Middle, no fragment");
			block.insert(block.end(), data.data() + it, data.data() + it + length);
			break;
		case TYPE_LAST:
			if (block.empty())
				return throwError("Last, no fragment");
			block.insert(block.end(), data.data() + it, data.data() + it + length);
			blocks.emplace_back(std::move(block));
			block.clear();
			break;
		default:
			return throwError(fmt::format("Unknown type {:d}", type));
		}
		it += length;
		prev_type = type;
	}

	for (auto & block : blocks)
	{
		if (block.size() < 12)
			continue;
		const uint8_t * ptr = block.data();
		auto sequence = endianess::fromLittle<uint64_t>(ptr);
		ptr += sizeof(sequence);
		auto count = endianess::fromLittle<uint32_t>(ptr);
		ptr += sizeof(count);
		while (ptr < block.data() + block.size())
		{
			auto tag = *ptr;
			ptr += sizeof(tag);
			std::vector<uint8_t> key;
			VectorData value;
			switch (tag)
			{
			case 0: // Delete
				{
					auto key_len = leveldb::read_varint<int32_t>(ptr);
					if (ptr + key_len > block.data() + block.size())
						continue;
					key.assign(ptr, ptr + key_len);
					ptr += key_len;
					value = {};
				}
				break;
			case 1: // Value
				{
					auto key_len = leveldb::read_varint<int32_t>(ptr);
					if (ptr + key_len > block.data() + block.size())
						continue;
					key.assign(ptr, ptr + key_len);
					ptr += key_len;
					auto val_len = leveldb::read_varint<int32_t>(ptr);
					if (ptr + val_len > block.data() + block.size())
						continue;
					value = VectorData(ptr, val_len);
					ptr += val_len;
				}
				break;
			default:
				return throwError(fmt::format("Unknown tag {:d}", tag));
			}
			visit(key, value);
		}
	}
	return 0;
}

} // namespace LevelDB

BlockParser::BlockParser(const uint8_t * begin, const uint8_t * end)
	: ptr(begin), end(end)
{
}
std::pair<std::vector<uint8_t>, LevelDB::VectorData> BlockParser::next()
{
	if (!has())
		return {};
	// Read next
	auto shared_bytes = leveldb::read_varint<uint32_t>(ptr);
	auto unshared_bytes = leveldb::read_varint<uint32_t>(ptr);
	auto value_length = leveldb::read_varint<uint32_t>(ptr);
	// Get key
	decltype(prev_key) key;
	key.reserve(shared_bytes + unshared_bytes);
	key.insert(key.end(), prev_key.begin(), prev_key.begin() + shared_bytes);
	key.insert(key.end(), ptr, ptr + unshared_bytes);
	ptr += unshared_bytes;
	// Get value
	LevelDB::VectorData data = {ptr, value_length};
	ptr += value_length;
	prev_key = key;
	return std::make_pair(key, data);
}
bool BlockParser::has() const
{
	return ptr != end;
}

inline std::tuple<uint64_t, uint64_t> read_block_handle(const uint8_t *& ptr)
{
	auto offset = leveldb::read_varint<uint64_t>(ptr);
	auto size = leveldb::read_varint<uint64_t>(ptr);
	return std::make_tuple(offset, size);
}

inline void skip_block_handle(const uint8_t *& ptr)
{
	leveldb::skip_varint(ptr);
	leveldb::skip_varint(ptr);
}

inline ChunkKey read_chunk_key(const std::vector<uint8_t> & data)
{
	/*
	chunkX: int32
	chunkZ: int32
	(dimension: int32) // Non-existant when dimension == 0
	type: int8
	(index: int8) // SubChunkPrefix
	// New since version 9
	?: int8 // 0 if type == 51 or 57, otherwise 1
	index?: int8 // There is some sort of iteration
	timestamp?: int16 // Values change rarely
	PADDING: int32 = 0
	*/
	auto it = data.data();
	ChunkKey key;
	key.x = endianess::fromLittle<int32_t>(it);
	it += sizeof(int32_t);
	key.z = endianess::fromLittle<int32_t>(it);
	it += sizeof(int32_t);
	if (data.size() > 18)
	{
		key.dimension = endianess::fromLittle<int32_t>(it);
		it += sizeof(int32_t);
	}
	key.type = *(it++);
	key.index = (data.size() % 2 == 0) ? *(it++) : -1;
	return key;
}

inline const uint8_t * get_block_end_pos(const std::vector<uint8_t> & block)
{
	auto num_restarts = endianess::fromLittle<uint32_t>(block.data() + block.size() - sizeof(uint32_t));
	auto size = block.size() - ((sizeof(uint32_t) * num_restarts) + sizeof(uint32_t));
	return block.data() + size;
}

/*std::vector<uint8_t> load_block(const std::vector<uint8_t> & data, uint64_t offset, uint64_t size)
{
	return load_block({data.data(), data.size()}, offset, size);
}*/

std::vector<uint8_t> load_block(LevelDB::VectorData data, uint64_t offset, uint64_t size)
{
	if (offset + size + 1 > data.size())
		return {};
	auto ptr = data.data() + offset;
	auto type = *(ptr + size);
	auto block = load_block_type(type, {ptr, size});
	return block;
}

inline std::vector<uint8_t> load_block_type(uint8_t type, const std::vector<uint8_t> & data)
{
	return load_block_type(type, {data.data(), data.size()});
}

std::vector<uint8_t> load_block_type(uint8_t type, LevelDB::VectorData data)
{
	switch (type)
	{
	case COMPRESSOR_RAW:
		return std::vector<uint8_t>{data.begin(), data.end()};
		break;
	case COMPRESSOR_ZLIB:
		return Compression::loadZLib(data);
		break;
	case COMPRESSOR_ZLIBRAW:
		return Compression::loadZLibRaw(data);
		break;
	case COMPRESSOR_SNAPPY:
		spdlog::error("Snappy not supported");
		break;
	case COMPRESSOR_ZSTD:
		spdlog::error("ZSTD not supported");
		break;
	default:
		spdlog::error("Unknown type: {:d}", type);
		break;
	}
	return {};
}
