/*
 * Difference from v16
 * - Wider section level, including negative position
 * - Heightmap 64 blocks offset
 * - Block states and palette are bundled together in a section
 * - Empty section, less data
 * - Nibbles were simplifed
 * - Changing several field names into snake-case
 */
#include "anvil/v18.hpp"

#include "anvil/limits.hpp"
#include "util/nibble.hpp"
#include "util/palette.hpp"

#include <algorithm>


bool anvil::V18::visit(const NBT::Tag & tag)
{
	// Get heightmaps
	if (heightmaps)
	{
		if (tag == NBT::TAG_End)
			heightmaps = false;
		else if (tag.isName("WORLD_SURFACE"))
		{
			auto & d = tag.get<NBT::NBTLongArray>();
			std::vector<int32_t> heightmap(SECTION_AREA);
			auto bits = d.size() / (SECTION_AREA / 64);
			MC16::nibbleCopy(d, heightmap, bits);
			std::for_each(heightmap.begin(), heightmap.end(), [](auto & a) { a -= 64; });
			chunk.setHeightMap(std::move(heightmap));
		}
	}
	// Get palette
	else if (palettes_left > 0)
	{
		// Finished with the compound, so copy over
		if (tag == NBT::TAG_End)
			--palettes_left;
		else if (tag.isName("Name"))
			palette.emplace_back(tag.get<NBT::NBTString>());
		else if (tag.isName("Properties"))
			return true;
	}
	// Get block states
	else if (block_states)
	{
		if (tag == NBT::TAG_End)
			block_states = false;
		else if (tag.isName("palette"))
		{
			palettes_left = tag.count();
			palette.reserve(std::size_t(palettes_left));
		}
		else if (tag.isName("data"))
			blocks = tag.get<NBT::NBTLongArray>();
	}
	// Get data
	else if (sections_left > 0)
	{
		// Finished with the compound, so copy over
		if (tag == NBT::TAG_End)
		{
			if (!palette.empty())
			{
				std::vector<uint16_t> _blocks;
				if (palette.size() == 1)
				{
					_blocks.resize(1);
				}
				else
				{
					_blocks.resize(SECTION_SIZE);
					auto bits = blocks.size() / (SECTION_SIZE / 64);
					MC16::nibbleCopy(blocks, _blocks, bits);
				}

				palette::translate(chunk, section, ns, _blocks, palette);
			}
			palette.clear();
			blocks = {};
			--sections_left;
		}
		else if (tag.isName("Y"))
		{
			// Apparently in some rare cases this can be an INT
			section.setY(
				tag.type() == NBT::TAG_Byte
				? tag.get<int8_t>()
				: tag.get<int32_t>());
		}
		else if (tag.isName("BlockLight"))
			section.setBlockLight(toVector(tag.get<NBT::NBTByteArray>()));
		else if (tag.isName("SkyLight"))
			section.setSkyLight(toVector(tag.get<NBT::NBTByteArray>()));
		else if (tag.isName("block_states"))
			block_states = true;
		else if (tag.isName("biomes"))
			return true;
	}
	else if (tag.isName("sections"))
	{
		sections_left = tag.count();
		section.clear();
	}
	else if (tag.isName("xPos"))
		chunk.setX(tag);
	else if (tag.isName("zPos"))
		chunk.setZ(tag);
	else if (tag.isName("yPos"))
		chunk.setY(tag);
	else if (tag.isName("Heightmaps"))
		heightmaps = true;
	// This is totally not interesting
	else if (tag.isName("blending_data"))
		return true;
	else if (tag.isName("block_entities"))
		return true;
	else if (tag.isName("block_ticks"))
		return true;
	else if (tag.isName("fluid_ticks"))
		return true;
	else if (tag.isName("structures"))
		return true;
	else if (tag.isName("CarvingMasks"))
		return true;
	else if (tag.isName("Entities"))
		return true;
	else if (tag.isName("Lights"))
		return true;
	else if (tag.isName("PostProcessing"))
		return true;

	return false;
}