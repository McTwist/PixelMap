#include "minecraft.hpp"

#include "format/alpha.hpp"
#include "format/region.hpp"
#include "format/leveldb.hpp"
#include "anvil/level.hpp"
#include "bedrock/level.hpp"
#include "bedrock/parse.hpp"
#include "platform.hpp"
#include "util/compression.hpp"

#include <fmt/format.h>

#include <set>
#include <unordered_set>
#include <filesystem>
#include <array>

namespace Minecraft
{

namespace JE
{

enum LevelVersion
{
	LEVEL_BETA = 19132,
	LEVEL_ANVIL = 19133,
};

// Get default folder
std::string getDefaultPath()
{
	std::string path;
#if defined(PLATFORM_WINDOWS)
	path = platform::path::join(
		platform::path::getenv("APPDATA"),
		".minecraft"
	);
#elif defined(PLATFORM_APPLE)
	path = platform::path::join(
		platform::path::getenv("HOME"),
		"Library",
		"Application Support",
		"minecraft"
	);
#elif defined(PLATFORM_LINUX)
	path = platform::path::join(
		platform::path::getenv("HOME"),
		".minecraft"
	);
#else
	#error "Not a supported platform"
#endif
	return path;
}

// Get all saves from default folder
std::vector<std::string> getDefaultPaths()
{
	auto path = getDefaultPath();
	path = platform::path::join(path, "saves");
	return getWorlds(std::move(path));
}

std::string getDimensionPath(const std::string & path, int32_t dimension)
{
	std::vector<std::string> p;
	p.reserve(3);
	p.emplace_back(path);
	if (dimension != 0)
		p.emplace_back(fmt::format("DIM{:d}", dimension));
	p.emplace_back("region");
	return platform::path::join(p);
}

// Get all saves from a path
std::vector<std::string> getWorlds(const std::string & path)
{
	std::vector<std::string> saves;
	if (!std::filesystem::is_directory(path))
		return saves;
	std::error_code ec;
	for (const auto & entry : std::filesystem::directory_iterator{path, ec})
	{
		if (!entry.is_directory())
			continue;
		saves.emplace_back(entry.path().string());
	}

	return saves;
}

// Get all dimensions from a path
std::vector<int32_t> getPathDimensions(const std::string & path)
{
	std::vector<int32_t> dimensions;
	if (!std::filesystem::is_directory(path))
		return dimensions;

	std::error_code ec;
	for (const auto & entry : std::filesystem::directory_iterator{path, ec})
	{
		if (!entry.is_directory())
			continue;
		auto name = entry.path().filename().string();
		// Overworld
		if (name == "region")
			dimensions.emplace_back(0);
		// All other dimensions
		else if (name.substr(0, 3) == "DIM")
		{
			try
			{
				dimensions.emplace_back(std::stoi(name.substr(3)));
			}
			catch(...)
			{
				// Do nothing
			}
		}
	}

	return dimensions;
}

std::shared_ptr<WorldInfo> getWorldInfo(const std::string & path)
{
	auto info = std::make_shared<WorldInfo>();
	region::RegionType type = region::RegionType::ANVIL;

	// Get all level info, if possible
	{
		anvil::Level level;
		std::vector<uint8_t> data = level.load(platform::path::join(path, "level.dat"));
		if (!data.empty())
		{
			NBT::Reader reader;
			data = Compression::loadGZip(data);
			if (!data.empty())
			{
				anvil::LevelReader levelReader(level);
				if (reader.parse(data, levelReader, NBT::Endianess::BIG) > 0)
				{
					info->game = Game::JAVA_EDITION;
					info->name = level.getName();
					info->seed = level.getSeed();
					info->ticks = level.getTicks();
					info->time = level.getTime();
					info->minecraftVersion = level.getVersionName();
					type = level.getVersion() == LEVEL_ANVIL ? region::RegionType::ANVIL : region::RegionType::BETA;
				}
			}
		}
	}

	auto dimensions = getPathDimensions(path);

	std::unordered_map<int32_t, std::string> names{{0, "Overworld"}, {-1, "Nether"}, {1, "The End"}};
	for (auto dimension : dimensions)
	{
		auto dimension_path = getDimensionPath(path, dimension);
		region::Region region(dimension_path, type);
		WorldInfo::DimensionInfo dim{"", dimension};
		for (auto file : region)
		{
			dim.amount_chunks += static_cast<decltype(dim.amount_chunks)>(file->getAmountChunks());
		}
		dim.name = names.find(dimension) == names.end() ? fmt::format("DIM{:d}", dimension) : names[dimension];

		if (dim.amount_chunks)
			info->dimensions.emplace_back(dim);
	}

	// Try alpha
	if (info->dimensions.empty())
	{
		for (auto dimension : dimensions)
		{
			std::string dimension_path;
			if (dimension != 0)
				dimension_path = platform::path::join(path, fmt::format("DIM{:d}", dimension));
			else
				dimension_path = path;
			alpha::Alpha alpha(dimension_path);
			alpha.begin();
			auto count = alpha.count();
			WorldInfo::DimensionInfo dim{"", dimension, count};
			dim.name = names.find(dimension) == names.end() ? fmt::format("DIM{:d}", dimension) : names[dimension];

			if (dim.amount_chunks)
				info->dimensions.emplace_back(dim);
		}
	}

	return info;
}

} // namespace JE

namespace BE
{

std::string getDefaultPath()
{
	std::string path;
#if defined(PLATFORM_WINDOWS)
	path = platform::path::join(
		platform::path::getenv("LOCALAPPDATA"),
		"Packages",
		"Microsoft.MinecraftUWP_8wekyb3d8bbwe",
		"LocalState",
		"games",
		"com.mojang"
	);
#elif defined(PLATFORM_APPLE)
#elif defined(PLATFORM_LINUX)
	// Note: Minecraft Bedrock does not exist on Linux,
	// but this is used both for development and if the
	// user transfers bedrock worlds to Linux.
	path = platform::path::join(
		platform::path::getenv("HOME"),
		".minecraftbe"
	);
#else
	#error "Not a supported platform"
#endif
	return path;
}

// Get all saves from default folder
std::vector<std::string> getDefaultPaths()
{
	auto path = getDefaultPath();
	path = platform::path::join(path, "minecraftWorlds");
	return getWorlds(std::move(path));
}

// Get all saves from a path
std::vector<std::string> getWorlds(const std::string & path)
{
	std::vector<std::string> saves;
	if (!std::filesystem::is_directory(path))
		return saves;

	std::error_code ec;
	for (const auto & entry : std::filesystem::directory_iterator{path, ec})
	{
		if (!entry.is_directory())
			continue;
		saves.emplace_back(entry.path().string());
	}

	return saves;
}

static void getWorldDimensions(const std::shared_ptr<WorldInfo> & info, const std::string & path)
{
	LevelDB::LevelDB ldb(path);
	LevelDB::LevelReader reader;
	std::unordered_map<int32_t, uint64_t> dims;

	for (auto file : ldb)
	{
		auto block = file->readAll();
		auto diff = reader.parse(block, [&dims](const std::vector<uint8_t> & key, const LevelDB::VectorData &){
			if (parse::mc::is_key_sub_chunk_prefix(key))
				return;
			parse::ChunkKey _key = parse::read_chunk_key(key);
			int32_t dim = _key.dimension;
			dims.try_emplace(dim, 1).first->second++;
		});
		if (diff < 0)
			return;
	}

	// Note: Find a better way to display dimension names
	std::unordered_map<int32_t, std::string> names{{0, "Overworld"}, {-1, "Nether"}, {1, "The End"}};
	for (auto [dimension, chunks] : dims)
	{
		auto name = names.find(dimension) == names.end() ? fmt::format("DIM{:d}", dimension) : names[dimension];
		info->dimensions.emplace_back(WorldInfo::DimensionInfo{std::move(name), dimension, chunks});
	}
	std::sort(info->dimensions.begin(), info->dimensions.end(), [](const auto & a, const auto & b) {
		return a.dimension < b.dimension;
	});
}

std::shared_ptr<WorldInfo> getWorldInfo(const std::string & path)
{
	auto info = std::make_shared<WorldInfo>();

	// Get all level info, if possible
	{
		bedrock::Level level;
		std::vector<uint8_t> data = level.load(platform::path::join(path, "level.dat"));
		if (!data.empty())
		{
			NBT::Reader reader;
			bedrock::LevelReader levelReader(level);
			if (reader.parse(data, levelReader, NBT::Endianess::LITTLE) > 0)
			{
				info->game = Game::BEDROCK_EDITION;
				info->name = level.getName();
				info->seed = level.getSeed();
				info->ticks = level.getTicks();
				info->time = level.getTime();
				info->minecraftVersion = level.getVersionName();
			}
		}
	}

	getWorldDimensions(info, platform::path::join(path, "db"));

	return info;
}

std::string getDefaultWorldPath(const std::string & world)
{
	auto path = getDefaultPath();
	path = platform::path::join(path, "minecraftWorlds", world);
	return path;
}

} // namespace BE

std::vector<std::string> getDefaultPaths()
{
	auto anvilPaths = JE::getDefaultPaths();
	auto bedrockPaths = BE::getDefaultPaths();
	decltype(anvilPaths) paths;
	paths.reserve(anvilPaths.size() + bedrockPaths.size());
	paths.insert(paths.end(), anvilPaths.begin(), anvilPaths.end());
	paths.insert(paths.end(), bedrockPaths.begin(), bedrockPaths.end());
	return paths;
}

std::shared_ptr<WorldInfo> getWorldInfo(const std::string & path)
{
	if (std::filesystem::is_directory(platform::path::join(path, "region")))
		return JE::getWorldInfo(path);
	else if (std::filesystem::is_directory(platform::path::join(path, "db")))
		return BE::getWorldInfo(path);
	// Not world folder, guess game version
	auto info = std::make_shared<WorldInfo>();

	// JE
	{
		WorldInfo::DimensionInfo dim{"", 0};

		// Anvil
		{
			region::Region region(path);
			for (auto file : region)
				dim.amount_chunks += static_cast<decltype(dim.amount_chunks)>(file->getAmountChunks());

			if (dim.amount_chunks)
			{
				info->game = Game::JAVA_EDITION;
				info->dimensions.emplace_back(dim);
				return info;
			}
		}

		// Beta
		{
			region::Region region(path, region::RegionType::BETA);
			for (auto file : region)
				dim.amount_chunks += static_cast<decltype(dim.amount_chunks)>(file->getAmountChunks());

			if (dim.amount_chunks)
			{
				info->game = Game::JAVA_EDITION;
				info->dimensions.emplace_back(dim);
				return info;
			}
		}

		// Alpha
		{
			alpha::Alpha alpha(path);
			alpha.begin();
			dim.amount_chunks = alpha.count();

			if (dim.amount_chunks)
			{
				info->game = Game::JAVA_EDITION;
				info->dimensions.emplace_back(dim);
				return info;
			}
		}
	}
	// BE
	{
		BE::getWorldDimensions(info, path);
		if (!info->dimensions.empty())
		{
			info->game = Game::BEDROCK_EDITION;
			return info;
		}
	}
	return info;
}

Game getPathGame(const std::string & path)
{
	if (!std::filesystem::is_directory(path))
		return Game::UNKNOWN;
	if (std::filesystem::is_directory(platform::path::join(path, "region")))
		return Game::JAVA_EDITION;
	else if (std::filesystem::is_directory(platform::path::join(path, "db")))
		return Game::BEDROCK_EDITION;
	std::error_code ec;
	// Not world folder, guess game version
	for (const auto & entry : std::filesystem::directory_iterator{path, ec})
	{
		if (entry.is_directory())
			continue;
		auto ext = entry.path().extension().string();
		if (ext == ".mca" || ext == ".mcr")
			return Game::JAVA_EDITION;
		if (ext == ".ldb" || ext == ".log")
			return Game::BEDROCK_EDITION;
	}
	// Alpha specific
	for (const auto & entry : std::filesystem::directory_iterator{path, ec})
	{
		if (!entry.is_directory())
			continue;
		for (const auto & e_rec : std::filesystem::recursive_directory_iterator{entry.path(), ec})
		{
			if (e_rec.is_directory())
				continue;
			auto ext = e_rec.path().extension().string();
			if (ext == ".dat")
				return Game::JAVA_EDITION;
		}
	}
	return Game::UNKNOWN;
}

SaveVersion determineSaveVersion(const std::string & path)
{
	std::error_code ec;
	std::string p = platform::path::join(path, "region");
	if (std::filesystem::is_directory(p))
	{
		for (const auto & entry : std::filesystem::directory_iterator{p, ec})
		{
			if (entry.is_directory())
				continue;
			auto ext = entry.path().extension().string();
			if (ext == ".mca")
				return SaveVersion::ANVIL;
			if (ext == ".mcr")
				return SaveVersion::BETA;
		}
	}
	p = platform::path::join(path, "db");
	if (std::filesystem::is_directory(p))
	{
		for (const auto & entry : std::filesystem::directory_iterator{p, ec})
		{
			if (entry.is_directory())
				continue;
			auto ext = entry.path().extension().string();
			if (ext == ".ldb" || ext == ".log")
				return SaveVersion::LEVELDB;
		}
	}
	// Not world folder, guess game version
	for (const auto & entry : std::filesystem::directory_iterator{path, ec})
	{
		if (entry.is_directory())
			continue;
		auto ext = entry.path().extension().string();
		if (ext == ".mca")
			return SaveVersion::ANVIL;
		if (ext == ".mcr")
			return SaveVersion::BETA;
		if (ext == ".ldb" || ext == ".log")
			return SaveVersion::LEVELDB;
	}
	// Alpha specific
	for (const auto & e1 : std::filesystem::directory_iterator{path, ec})
	{
		if (!e1.is_directory())
			continue;
		for (const auto & e2 : std::filesystem::recursive_directory_iterator{e1.path(), ec})
		{
			if (!e2.is_directory())
				continue;
			for (const auto & e3 : std::filesystem::recursive_directory_iterator{e2.path(), ec})
			{
				if (e3.is_directory())
					continue;
				auto ext = e3.path().extension().string();
				if (ext == ".dat")
					return SaveVersion::ALPHA;
			}
		}
	}
	return SaveVersion::UNKNOWN;
}

} // namespace Minecraft
