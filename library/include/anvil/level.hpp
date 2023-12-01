#pragma once
#ifndef ANVIL_LEVEL_H
#define ANVIL_LEVEL_H

#include "format/nbt.hpp"

#include <string>
#include <vector>

namespace anvil
{

// Level with all needed informations
class Level
{
public:
	Level();

	std::vector<uint8_t> load(const std::string & path);

	void setVersion(uint32_t version);
	void setName(std::string name);
	void setSeed(int64_t seed);
	void setTick(int64_t tick);
	void setTime(int64_t time);
	void setVersionId(int32_t id);
	void setVersionName(std::string name);

	uint32_t getVersion() const { return version; }
	std::string getName() const { return levelName; }
	int64_t getSeed() const { return randomSeed; }
	int64_t getTicks() const { return tick; }
	int64_t getTime() const { return time; }
	int32_t getVersionId() const { return minecraftVersionId; }
	std::string getVersionName() const { return minecraftVersionName; }

private:
	int32_t version = 0;
	std::string levelName;
	int64_t randomSeed = 0;
	int64_t tick = 0;
	int64_t time = 0;
	int32_t minecraftVersionId = 0;
	std::string minecraftVersionName;
};

// A class based on level reader
class LevelReader : public NBT::Visitor
{
public:
	LevelReader(Level & level);

	bool visit(const NBT::Value & value);
	bool visit(const NBT::Tag & tag);

private:
	Level & level;

	bool minecraftVersion = false, minecraftSeed = false;
};

} // namespace anvil

#endif // ANVIL_LEVEL_H
