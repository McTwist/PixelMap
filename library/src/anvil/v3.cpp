#include "anvil/v3.hpp"

#include "anvil/limits.hpp"
#include "util/nibble.hpp"
#include "util/palette.hpp"

anvil::V3::V3(Chunk & chunk) :
	V(chunk)
{
	std::fill_n(id, palette::ID_SIZE, BLOCK_ID_MAX);
}


bool anvil::V3::visit(const NBT::Tag & tag)
{
	// Get data
	if (sections_left > 0)
	{
		// Finished with the compound, so copy over
		if (tag == NBT::TAG_End)
		{
			palette::translate(chunk, section, id, blocks);
			--sections_left;
		}
		else if (tag.isName("Y"))
			section.setY(tag);
		else if (tag.isName("BlockLight"))
			section.setBlockLight(tag);
		else if (tag.isName("SkyLight"))
			section.setSkyLight(tag);
		else if (tag.isName("Blocks"))
		{
			auto & d = tag.get<NBT::NBTByteArray>();
			if (blocks.empty())
				blocks.resize(SECTION_SIZE);
			for (auto i = 0U; i < d.size(); ++i)
				blocks[i] = (blocks[i] & 0xFF00) | uint16_t(d[i]);
		}
		// Block ID extension
		else if (tag.isName("Add"))
		{
			auto & d = tag.get<NBT::NBTByteArray>();
			if (blocks.empty())
				blocks.resize(SECTION_SIZE);
			for (auto i = 0U; i < d.size(); ++i)
				blocks[i] = (blocks[i] & 0xF0FF) | uint16_t(nibble4(d, i) << 8);
		}
		// Specific data values
		else if (tag.isName("Data"))
		{
			auto & d = tag.get<NBT::NBTByteArray>();
			if (blocks.empty())
				blocks.resize(SECTION_SIZE);
			auto s = d.size() << 1;
			for (auto i = 0U; i < d.size() && i < s; ++i)
				blocks[i] = (blocks[i] & 0x0FFF) | uint16_t(nibble4(d, i) << 12);
		}
	}
	else if (tag.isName("Sections"))
	{
		sections_left = tag.count();
		section.clear();
	}
	else if (tag.isName("xPos"))
		chunk.setX(tag);
	else if (tag.isName("zPos"))
		chunk.setZ(tag);
	else if (tag.isName("HeightMap"))
		chunk.setHeightMap(tag);
	// This is totally not interesting
	else if (tag.isName("Entities"))
		return true;
	else if (tag.isName("PostProcessing"))
		return true;
	else if (tag.isName("TileEntities"))
		return true;
	else if (tag.isName("TileTicks"))
		return true;

	return false;
}

