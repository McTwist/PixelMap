#include "alpha/v.hpp"

#include "anvil/limits.hpp"
#include "alpha/limits.hpp"
#include "util/nibble.hpp"
#include "util/palette.hpp"


alpha::V::V(Chunk & _chunk)
	: chunk(_chunk)
{
	id.fill(BLOCK_ID_MAX);
	chunk.setPaletteType(PT_BLOCKID);
}

bool alpha::V::visit(const NBT::Value & value)
{
	(void)value;
	return false;
}

bool alpha::V::visit(const NBT::Tag & tag)
{
	if (level)
	{
		if (tag == NBT::TAG_End)
		{
			for (int i = 0; i < 8; ++i)
			{
				sections[i].setY(i);
				palette::translate(chunk, sections[i], id, blocks[i]);
			}
			level = false;
		}
		else if (tag.isName("xPos"))
			chunk.setX(tag);
		else if (tag.isName("zPos"))
			chunk.setZ(tag);
		else if (tag.isName("Blocks"))
		{
			auto source = tag.get<NBT::NBTByteArray>();
			for (auto n = 0; n < 8; ++n)
			{
				auto & b = blocks[n];
				if (b.empty())
					b.resize(SECTION_SIZE);
				for (auto x = 0; x < SECTION_X; ++x)
					for (auto y = 0; y < SECTION_Y; ++y)
						for (auto z = 0; z < SECTION_Z; ++z)
							b[(y << 8) | (z << 4) | x] = (b[(y << 8) | (z << 4) | x] & 0xFF00) | uint16_t(source[(x << 11) | (z << 7) | (y + (n << 4))]);
			}
		}
		else if (tag.isName("Data"))
		{
			auto source = tag.get<NBT::NBTByteArray>();
			for (auto n = 0; n < 8; ++n)
			{
				auto & b = blocks[n];
				if (b.empty())
					b.resize(SECTION_SIZE);
				for (auto x = 0; x < SECTION_X; ++x)
					for (auto y = 0; y < SECTION_Y; ++y)
						for (auto z = 0; z < SECTION_Z; ++z)
							b[(y << 8) | (z << 4) | x] = (b[(y << 8) | (z << 4) | x] & 0x0FFF) | uint16_t(nibble4(source, (x << 11) | (z << 7) | (y + (n << 4))) << 12);
			}
		}
		else if (tag.isName("BlockLight"))
		{
			auto source = tag.get<NBT::NBTByteArray>();
			for (auto n = 0; n < 8; ++n)
			{
				auto & section = sections[n];
				std::vector<int8_t> b;
				b.resize(SECTION_SIZE / 2);
				for (auto x = 0; x < SECTION_X; ++x)
					for (auto y = 0; y < SECTION_Y / 2; ++y)
						for (auto z = 0; z < SECTION_Z; ++z)
							b[(y << 8) | (z << 4) | x] = source[(x << 11) | (z << 7) | (y + (n << 3))];
				section.setBlockLight(b);
			}
		}
		else if (tag.isName("SkyLight"))
		{
			auto source = tag.get<NBT::NBTByteArray>();
			for (auto n = 0; n < 8; ++n)
			{
				auto & section = sections[n];
				std::vector<int8_t> b;
				b.resize(SECTION_SIZE / 2);
				for (auto x = 0; x < SECTION_X; ++x)
					for (auto y = 0; y < SECTION_Y / 2; ++y)
						for (auto z = 0; z < SECTION_Z; ++z)
							b[(y << 8) | (z << 4) | x] = source[(x << 11) | (z << 7) | (y + (n << 3))];
				section.setSkyLight(b);
			}
		}
		else if (tag.isName("HeightMap"))
		{
			auto & src = tag.get<NBT::NBTByteArray>();
			std::vector<int32_t> heightmap(Minecraft::chunkWidth() * Minecraft::chunkWidth());
			std::transform(src.begin(), src.end(), heightmap.begin(), [](int8_t a) { return int32_t(a); });
			chunk.setHeightMap(std::move(heightmap));
		}
	}
	else if (tag.isName(""))
		return false;
	else if (tag.isName("Level"))
	{
		level = true;
		return false;
	}
	
	return true;
}
