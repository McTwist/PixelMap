#include "alpha/v.hpp"

#include "anvil/limits.hpp"
#include "alpha/limits.hpp"
#include "util/nibble.hpp"
#include "util/palette.hpp"

#include <spdlog/spdlog.h>

#include <functional>


template<typename T>
void convert_alpha([[maybe_unused]] const NBT::NBTByteArray & src, std::vector<T> & dst, std::size_t n, const std::function<T(T, std::size_t)> & convert)
{
	assert(src.size() == dst.size());
	for (auto x = 0; x < SECTION_X; ++x)
		for (auto y = 0; y < SECTION_Y; ++y)
			for (auto z = 0; z < SECTION_Z; ++z)
				dst[(y << 8) | (z << 4) | x] = convert(dst[(y << 8) | (z << 4) | x], (x << 11) | (z << 7) | (y + (n << 4)));
}


alpha::V::V(Chunk & _chunk)
	: chunk(_chunk)
{
	id.fill(BLOCK_ID_MAX);
	chunk.setPaletteType(PaletteType::BLOCKID);
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
				palette::translate(chunk, std::move(sections[i]), id, blocks[i]);
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
			for (std::size_t n = 0; n < 8; ++n)
			{
				auto & b = blocks[n];
				if (b.empty())
					b.resize(SECTION_SIZE);
				convert_alpha<uint16_t>(source, b, n, [&source](uint16_t d, std::size_t i) -> uint16_t {
					return (d & 0xFF00) | uint16_t(source[i]);
				});
			}
		}
		else if (tag.isName("Data"))
		{
			auto source = tag.get<NBT::NBTByteArray>();
			for (std::size_t n = 0; n < 8; ++n)
			{
				auto & b = blocks[n];
				if (b.empty())
					b.resize(SECTION_SIZE);
				convert_alpha<uint16_t>(source, b, n, [&source](uint16_t d, std::size_t i) -> uint16_t {
					return (d & 0x0FFF) | uint16_t(nibble4(source, i) << 12);
				});
			}
		}
		else if (tag.isName("BlockLight"))
		{
			auto source = tag.get<NBT::NBTByteArray>();
			for (std::size_t n = 0; n < 8; ++n)
			{
				auto & section = sections[n];
				std::vector<uint8_t> b;
				b.resize(SECTION_SIZE);
				convert_alpha<uint8_t>(source, b, n, [&source](uint8_t, std::size_t i) -> uint8_t {
					return nibble4(source, i);
				});
				section.setBlockLight(b);
			}
		}
		else if (tag.isName("SkyLight"))
		{
			auto source = tag.get<NBT::NBTByteArray>();
			for (std::size_t n = 0; n < 8; ++n)
			{
				auto & section = sections[n];
				std::vector<uint8_t> b;
				b.resize(SECTION_SIZE);
				convert_alpha<uint8_t>(source, b, n, [&source](uint8_t, std::size_t i) -> uint8_t {
					return nibble4(source, i);
				});
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
