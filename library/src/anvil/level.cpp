#include "anvil/level.hpp"

#include "platform.hpp"

#include <fstream>

namespace anvil
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

	in.seekg(0, std::ios::end);
	out.resize(in.tellg());
	in.seekg(0, std::ios::beg);
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
void Level::setVersionId(int32_t id)
{
	minecraftVersionId = id;
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
	(void)value;
	return false;
}

bool LevelReader::visit(const NBT::Tag & tag)
{
	// Minecraft version
	if (minecraftVersion)
	{
		if (tag == NBT::TAG_End)
			minecraftVersion = false;
		else if (tag.isName("Id"))
			level.setVersionId(tag);
		else if (tag.isName("Name"))
			level.setVersionName(tag.get<NBT::NBTString>());
		return false;
	}
	// Minecraft seed
	else if (minecraftSeed)
	{
		if (tag == NBT::TAG_End)
			minecraftSeed = false;
		else if (tag.isName("seed"))
			level.setSeed(tag);
		return false;
	}
	// Root tag
	else if (tag.isName(""))
		return false;
	// Data tag
	else if (tag.isName("Data"))
		return false;
	else if (tag.isName("version"))
	{
		level.setVersion(tag);
		return false;
	}
	else if (tag.isName("LevelName"))
	{
		level.setName(tag.get<NBT::NBTString>());
		return false;
	}
	else if (tag.isName("RandomSeed"))
	{
		level.setSeed(tag);
		return false;
	}
	else if (tag.isName("Time"))
	{
		level.setTick(tag);
		return false;
	}
	else if (tag.isName("DayTime"))
	{
		level.setTime(tag);
		return false;
	}
	else if (tag.isName("Version"))
	{
		minecraftVersion = true;
		return false;
	}
	else if (tag.isName("WorldGenSettings"))
	{
		minecraftSeed = true;
		return false;
	}

	// Ignore all else
	return true;
}

}
