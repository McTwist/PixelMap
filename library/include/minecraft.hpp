#pragma once
#ifndef MINECRAFT_HPP
#define MINECRAFT_HPP

#include <string>
#include <vector>
#include <tuple>
#include <memory>

/**
 * @brief Collection of Minecraft functions
 * This collection is specifically aimed at Minecraft for easier access
 * between platforms. It also includes rudimentary functionality to get
 * more information about a certain world.
 */
namespace Minecraft
{
	/**
	 * @brief List of available game versions
	 */
	enum Game
	{
		GAME_UNKNOWN,
		GAME_JAVA_EDITION,
		GAME_BEDROCK_EDITION,
		GAME_POCKET_EDITION = GAME_BEDROCK_EDITION,
	};

	/**
	 * @brief Stores world info about a certain path
	 * @variable game The game version of the world
	 * @variable name The real name of the world
	 * @variable seed The seed to generate the world
	 * @variable ticks The amount of ticks since start of the world
	 * @variable time The time of the day
	 * @variable minecraftVersion If available, the version of Minecraft
	 * @variable dimensions A list of all dimensions available
	 */
	struct WorldInfo
	{
		Game game;
		std::string name;
		int64_t seed = 0;
		int64_t ticks = 0;
		int64_t time = 0;
		std::string minecraftVersion;

		/**
		 * @brief Information about a dimension
		 * @variable name The name of the dimension (Only supports default dimensions)
		 * @variable dimension The dimension id
		 * @variable amount_chunks The total amount of chunks available
		 */
		struct DimensionInfo
		{
			std::string name;
			int32_t dimension;
			uint64_t amount_chunks = 0;
		};
		std::vector<DimensionInfo> dimensions;
	};

	namespace JE
	{
		/**
		 * @brief Get default minecraft java path
		 * @return The path to the default path
		 */
		std::string getDefaultPath();

		/**
		 * @brief Get the default minecraft java paths
		 * @return A list of paths for the worlds available in the default path
		 */
		std::vector<std::string> getDefaultPaths();

		/**
		 * @brief Get the dimension path from path and dimension id
		 * @param path The path of the world
		 * @param dimension The dimension id
		 * @return Path to dimension
		 */
		std::string getDimensionPath(const std::string & path, int32_t dimension);

		/**
		 * @brief Get all worlds from a path
		 * @param path The path to look for worlds
		 * @return A list of worlds folder names available in the path
		 */
		std::vector<std::string> getWorlds(const std::string & path);

		/**
		 * @brief Get info about a path
		 * @param path The path to the world
		 * @return The info containing the information about the world
		 */
		std::shared_ptr<WorldInfo> getWorldInfo(const std::string & path);
	}

	namespace BE
	{
		/**
		 * @brief Get default minecraft bedrock path
		 * @return The path to the default path
		 */
		std::string getDefaultPath();

		/**
		 * @brief Get the default minecraft bedrock paths
		 * @return A list of paths for the worlds available in the default path
		 */
		std::vector<std::string> getDefaultPaths();

		/**
		 * @brief Get all worlds from a path
		 * @param path The path to look for worlds
		 * @return A list of worlds folder names available in the path
		 */
		std::vector<std::string> getWorlds(const std::string & path);

		/**
		 * @brief Get info about a path
		 * @param path The path to the world
		 * @return The info containing the information about the world
		 */
		std::shared_ptr<WorldInfo> getWorldInfo(const std::string & path);

		/**
		 * @brief Get default world path
		 * @param world The world folder name
		 * @return A path of the default world by name
		 */
		std::string getDefaultWorldPath(const std::string & world);
	}

	/**
	 * @brief Get the default minecraft paths
	 * @return A list of paths for the worlds available in the default path
	 */
	std::vector<std::string> getDefaultPaths();

	/**
	 * @brief Get info about a path
	 * @param path The path to the world
	 * @return The info containing the information about the world
	 */
	std::shared_ptr<WorldInfo> getWorldInfo(const std::string & path);

	/**
	 * @brief Get the path Game version
	 * @param path The path to check
	 * @return The version of the Game
	 */
	Game getPathGame(const std::string & path);

	/**
	 * @brief List of available save versions
	 */
	enum SaveVersion
	{
		SAVE_MCREGION,
		SAVE_ANVIL,
		SAVE_LEVELDB
	};

	// Width of chunk
	constexpr uint32_t chunkWidth();
	// Height of chunk
	constexpr uint32_t worldHeight(SaveVersion);
	// Height of section
	constexpr uint32_t sectionHeight(SaveVersion);
	// Amount of chunks for with in region
	constexpr uint32_t regionWidth();
	// Amount of sections in a chunk
	constexpr uint32_t sectionCount(SaveVersion);
	// Amount of chunks in a region
	constexpr uint32_t regionChunkCount();

	/*
	 * Implementations
	 */

	constexpr uint32_t chunkWidth()
	{
		return 16;
	}

	constexpr uint32_t worldHeight(SaveVersion version)
	{
		return sectionHeight(version) * sectionCount(version);
	}

	constexpr uint32_t sectionHeight(SaveVersion version)
	{
		switch (version)
		{
		case SAVE_MCREGION:
			return 128;
		case SAVE_ANVIL:
		case SAVE_LEVELDB:
			return 16;
		}
		return 0;
	}

	constexpr uint32_t regionWidth()
	{
		return 32;
	}

	constexpr uint32_t sectionCount(SaveVersion version)
	{
		switch (version)
		{
		case SAVE_MCREGION:
			// Has no sections, but assume one for simplicity
			return 1;
		case SAVE_ANVIL:
		case SAVE_LEVELDB:
			return 16;
		}
		return 0;
	}

	constexpr uint32_t regionChunkCount()
	{
		return regionWidth() * regionWidth();
	}
}

#endif // MINECRAFT_HPP
