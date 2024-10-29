#include "bedrock/level.hpp"

#include "platform.hpp"
#include "util/endianess.hpp"

#include <fstream>

namespace bedrock
{

Level::Level()
	: minecraftVersionName("N/A")
{
}

std::vector<uint8_t> Level::load(const std::string &path)
{
	std::vector<uint8_t> out;
	std::ifstream in;
	in.open(path.c_str(), std::ios::in | std::ios::binary);
	if (!in.is_open())
		return out;

	uint8_t buffer[8];
	in.read(reinterpret_cast<char *>(buffer), sizeof(int32_t) * 2);
	endianess::fromLittle<int32_t>(buffer); // Chunk version?
	auto _length = endianess::fromLittle<int32_t>(buffer + sizeof(int32_t));
	//std::cerr << _version << ", " << _length << std::endl;
	out.resize(_length);
	in.read(reinterpret_cast<char *>(out.data()), out.size());
	in.close();

	return out;
}

void Level::setVersion(uint32_t _version)
{
	version = _version;
}
void Level::setName(std::string name)
{
	levelName = name;
}
void Level::setSeed(int64_t seed)
{
	randomSeed = seed;
}
void Level::setTick(int64_t _tick)
{
	tick = _tick;
}
void Level::setTime(int64_t _time)
{
	time = _time;
}
void Level::addVersionId(int32_t id)
{
	minecraftVersionId.push_back(id);
}
void Level::setVersionName(std::string name)
{
	minecraftVersionName = name;
}

LevelReader::LevelReader(Level & level)
	: level(level)
{
}

bool LevelReader::visit(const NBT::Value & value)
{
	if (lastOpenedWithVersion > 0)
	{
		level.addVersionId(value);
		--lastOpenedWithVersion;
	}
	return false;
}

bool LevelReader::visit(const NBT::Tag & tag)
{
	// Root tag
	if (tag.isName(""))
		return false;
	else if (tag.isName("WorldVersion"))
		level.setVersion(tag);
	else if (tag.isName("LevelName"))
		level.setName(std::string{tag.get<NBT::NBTString>()});
	else if (tag.isName("RandomSeed"))
		level.setSeed(tag);
	else if (tag.isName("Time"))
		level.setTime(tag);
	else if (tag.isName("currentTick"))
		level.setTick(tag);
	else if (tag.isName("lastOpenedWithVersion"))
		lastOpenedWithVersion = tag.count();
	else if (tag.isName("InventoryVersion"))
		level.setVersionName(tag);
	else
		// Ignore all else
		return true;

	return false;
}

}
