#include "bedrock/v.hpp"

#include "bedrock/limits.hpp"
#include "bedrock/parse.hpp"
#include "format/varint.hpp"
#include "format/nbt.hpp"
#include "util/endianess.hpp"
#include "util/nibble.hpp"
#include "util/palette.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h> // fmt

#include <algorithm>

// https://minecraft.wiki/w/Bedrock_Edition_level_format#Chunk_key_format
// https://gist.github.com/Tomcc/a96af509e275b1af483b25c543cfbf37#the-new-subchunk-format
// https://learn.microsoft.com/en-us/minecraft/creator/documents/actorstorage#non-actor-data-chunk-key-ids
// Note: These sources deviate and it is not a given that either of them are correct.
enum ValueType : uint8_t
{
	TYPE_Data3D = 43,
		// Data: binary
		// heightmap(256*2 bytes)
		// biome_data(same as palette in SubChunkPrefix; unknown if this is true)
	TYPE_Version = 44,
	TYPE_Data2D = 45, // < 1.18.0
	TYPE_Data2DLegacy = 46, // < 1.0.0
	TYPE_SubChunkPrefix = 47,
		// Data: binary
		// version(byte) = 9
		// num_storage_blocks(byte)
		// chunkY(byte) = key.index
		// storage_block(list : num_storage_blocks)
		//   header(byte) (7 bits = bits_per_block; 1 bit : serialized = 0)
		//   block_states(nibble(int : bits_per_block) : 4096)
		//   palette_size(int)
		//   palette(list : palette_size): NBT
		//     name: String
		//     Val: String
	TYPE_LegacyTerrain = 48, // < 1.0.0
	TYPE_BlockEntity = 49,
	TYPE_Entity = 50,
	TYPE_PendingTicks = 51,
	TYPE_LegacyBlockExtraData = 52, // < 1.2.13
	TYPE_BiomeState = 53,
	TYPE_FinalizedState = 54,
	TYPE_ConversionData = 55,
	TYPE_BorderBlocks = 56, // Education Edition
	TYPE_HardcodedSpawners = 57,
	TYPE_RandomTicks = 58,
	TYPE_Checksums = 59, // < 1.18.0
	TYPE_GenerationSeed = 60,
	TYPE_GeneratedPreCavesAndCliffsBlending = 61, // Not used
	TYPE_BlendingBiomeHeight = 62, // Not used
	TYPE_MetaDataHash = 63,
		// Data: [0x27, 0x77, 0xfc, 0xc3, 0xd5, 0xc6, 0x75, 0x8d]
	TYPE_BlendingData = 64,
		// Data: [0x0, 0x3]
	TYPE_ActorDigestVersion = 65,
		// Data: [0x0]
	TYPE_WeatherData = 97,
		// Data: NBT
		// list: List(Compound)
		//   id: Byte
		//   snowAccumulation: Float
	TYPE_LimboEntities = 100,
		// Data: NBT
		// data:
		//   LimboEntities: List
	TYPE_ScenarioData = 114,
		// Data: NBT
		// Criteria: List
		// DisplayObjectives: List
		// Entries: List
		// LastUniqueID: Long
		// Objectives: Long
	TYPE_OptionsData = 115,
		// Data: NBT
		// events_enabled: Bool
		// minecraft:ender_dragon_event: Bool
		// minecraft:pillager_patrols_event: Bool
		// minecraft:wandering_trader_event: Bool
	TYPE_AutonomousEntityList = 117,
		// Data: NBT
		// AutonomousEntityList: List
	TYPE_LegacyVersion = 118, // < 1.16.100
};

/*
SubChunkPrefix:
version: 0,2,3,4,5,6,7
{
	version: uint8_t
	blocks: uint8_t[4096]
	data: uint8_t[4096]
}
version: 1
{
	version: uint8_t = 1
	storage_version: uint8_t:1(lsb)
	bits_per_blocks: uint8_t:7
	block_states_index: uint8_t[] // nibble
	palette_size: uint32_t
	palette: nbt[palette_size]
	block_states: nbt[] // XYZ(prev) -> YZX(now)
}
version: 8
{
	version: uint8_t = 8
	num_storage_blocks: uint8_t // Usually 1, omitted if version == 1
	storage_blocks: [num_storage_blocks] {
		storage_version: uint8_t:1(lsb)
		bits_per_blocks: uint8_t:7
		block_states_index: uint8_t[] // nibble
		palette_size: uint32_t
		palette: nbt[palette_size]
		block_states: nbt[] // YZX
	}
}
version: 9
{
	version: uint8_t = 9
	num_storage_blocks: uint8_t // Usually 1, omitted if version == 1
	chunk_y: int8_t
	storage_blocks: [num_storage_blocks] {
		storage_version: uint8_t:1(lsb)
		bits_per_blocks: uint8_t:7
		block_states_index: uint8_t[] // nibble
		palette_size: uint32_t
		palette: nbt[palette_size]
		block_states: nbt[] // YZX
	}
	extra: nbt {
		name: string
		states: object
		version: int
	}
}
*/

static void read_Data3D(bedrock::World & world, const parse::ChunkKey & key, const LevelDB::VectorData & data);
static void read_SubChunkPrefix(bedrock::World & world, std::unordered_map<std::string, uint16_t> & ns, const parse::ChunkKey & key, const LevelDB::VectorData & data);

bedrock::V::V(World & _world)
	: world(_world)
{
}

void bedrock::V::visit(const std::vector<uint8_t> & key, const LevelDB::VectorData & data)
{
	if (data.empty())
		return;
	if (parse::mc::is_key_sub_chunk_prefix(key))
		return;
	
	auto chunk_key = parse::read_chunk_key(key);

	if (chunk_key.dimension != world.getDimension())
		return;

	switch (chunk_key.type)
	{
	case TYPE_Data3D: read_Data3D(world, chunk_key, data); break;
	case TYPE_Version: break; // TODO: Use to determine version. Pre-parse?
	case TYPE_Data2D: break; // Legacy
	case TYPE_Data2DLegacy: break; // Legacy
	case TYPE_SubChunkPrefix: read_SubChunkPrefix(world, chunk_ns[{chunk_key.x, chunk_key.z}], chunk_key, data); break;
	case TYPE_LegacyTerrain: break; // Legacy
	case TYPE_BlockEntity: break; // TODO: Use for live map
	case TYPE_Entity: break; // TODO: Use for live map
	case TYPE_PendingTicks: break; // Unused
	case TYPE_LegacyBlockExtraData: break; // Legacy
	case TYPE_BiomeState: break; // Unknown
	case TYPE_FinalizedState: break; // Unused
	case TYPE_ConversionData: break; // Legacy
	case TYPE_BorderBlocks: break; // Education
	case TYPE_HardcodedSpawners: break; // Unused
	case TYPE_RandomTicks: break; // Unused
	case TYPE_Checksums: break; // Legacy
	case TYPE_BlendingBiomeHeight: break; // Unused
	case TYPE_BlendingData: break; // Unused
	case TYPE_ActorDigestVersion: break; // Unused
	case TYPE_WeatherData: break; // Unused
	case TYPE_LimboEntities: break; // Unused
	case TYPE_ScenarioData: break; // Unused
	case TYPE_OptionsData: break; // Unused
	case TYPE_LegacyVersion: break; // Legacy
	default:
		spdlog::debug("Found unknown type: {:d} {:d}", chunk_key.type, data.size());
		break;
	}
}

void read_Data3D(bedrock::World & world, const parse::ChunkKey & key, const LevelDB::VectorData & data)
{
	Chunk & chunk = world.getChunk(key.x, key.z);
	auto ptr = reinterpret_cast<const uint16_t *>(data.data());
	std::vector<int32_t> heightmap(256);
	std::transform(ptr, ptr+256, heightmap.begin(),
		[](auto & v) {
			return endianess::fromLittle<int16_t>(reinterpret_cast<const uint8_t *>(&v)) - 64;
		});
	chunk.setHeightMap(heightmap);
	ptr += 256;
	// Note: Biome format is undefined, but maybe use it later?
}

// Note: Split up depending on version
void read_SubChunkPrefix(bedrock::World & world, std::unordered_map<std::string, uint16_t> & ns, const parse::ChunkKey & key, const LevelDB::VectorData & data)
{
	Chunk & chunk = world.getChunk(key.x, key.z);
	chunk.setX(key.x);
	chunk.setZ(key.z);
	chunk.setPaletteType(PaletteType::NAMESPACEID);
	auto ptr = data.data();
	auto version = *(ptr++);
	uint8_t num_storage_blocks = 1;
	auto chunky = key.index;

	switch (version)
	{
	// 1.2
	case 0:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
	case 7:
		// Note: This is assumed from pre-flattening, but will probably never be used
		{
			std::vector<uint16_t> blocks;
			// Note: Does not store block ids over several sub-chunks, resulting in bigger palette
			std::array<uint16_t, palette::ID_SIZE> id{BLOCK_ID_MAX};
			blocks.resize(SECTION_SIZE);
			for (auto i = 0U; i < SECTION_SIZE; ++i)
				blocks[i] = (blocks[i] & 0xFF00) | uint16_t(ptr[i]);
			ptr += SECTION_SIZE;
			VectorView<const uint8_t> data{ptr, SECTION_SIZE / 2};
			auto s = SECTION_SIZE;
			for (auto i = 0U; i < data.size() && i < s; ++i)
				blocks[i] = (blocks[i] & 0x0FFF) | uint16_t(nibble4(data, i) << 12);
			SectionData section;
			section.setY(chunky);
			section.setBlockOrder(BlockOrder::XZY);
			palette::translate(chunk, std::move(section), id, blocks);
		}
		break;
	// Aquatic
	case 9:
	case 8:
		num_storage_blocks = *(ptr++);
		if (version >= 9)
			//chunky = *((int8_t *)ptr++);
			ptr++; // sub_chunk_index == chunky
		[[fallthrough]];
	// Palettized
	case 1:
		for (int n = 0; n < num_storage_blocks; ++n)
		{
			auto s = *(ptr++);
			uint8_t serialized = s & 1; // Always 0
			if (serialized == 1)
				continue;
			auto bits_per_block = s >> 1;
			if (bits_per_block == 0)
				continue;
			auto parts = 32 / bits_per_block; // int size
			std::size_t size = (4095 + parts) / parts; // byte size
			std::vector<uint16_t> block_states_index(4096);
			auto _states = reinterpret_cast<const uint32_t *>(ptr);
			VectorView<uint16_t> _block_states_index{block_states_index.data(), block_states_index.size()};
			MC16::nibbleCopy(VectorView<const uint32_t>{_states, size}, _block_states_index, bits_per_block);
			ptr += size * sizeof(uint32_t);
			auto palette_size = endianess::fromLittle<uint32_t>(ptr);
			ptr += sizeof(palette_size);
			// NBT traversal
			NBT::Reader reader;
			std::vector<std::string> _palette;
			_palette.reserve(palette_size);
			for (decltype(palette_size) j = 0; j < palette_size; ++j)
			{
				if (data.size() <= std::size_t(std::distance(data.data(), ptr)))
				{
					spdlog::error("Iteration has gone past size");
					return;
				}
				auto _size = data.size() - std::distance(data.data(), ptr);
				auto _diff = reader.parse({const_cast<uint8_t *>(ptr), _size}, [&_palette](const NBT::Tag & tag) {
					if (tag.isName("name"))
						_palette.emplace_back(tag.get<NBT::NBTString>());
					return false;
				}, [](const auto &) { return false; }, NBT::Endianess::LITTLE);
				if (_diff < 0)
				{
					spdlog::error(reader.getError());
					return;
				}
				ptr += _diff;
			}
			SectionData section;
			section.setY(chunky);
			section.setBlockOrder(BlockOrder::XZY);
			palette::translate(chunk, std::move(section), ns, block_states_index, _palette);
		}
		// Ignore the extra data
		break;
	default:
		spdlog::error("Unknown SubChunkPrefix version: {:d}", version);
	}
}
