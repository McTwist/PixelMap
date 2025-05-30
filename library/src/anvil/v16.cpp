/*
 * Difference from v13
 * - Change in nibble structure, less compressed
 */
#include "anvil/v16.hpp"

#include "anvil/limits.hpp"
#include "util/nibble.hpp"
#include "util/palette.hpp"


bool anvil::V16::visit(const NBT::Tag & tag)
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
	// Get data
	else if (sections_left > 0)
	{
		// Finished with the compound, so copy over
		if (tag == NBT::TAG_End)
		{
			if (!palette.empty() && !blocks.empty())
				palette::translate(chunk, std::move(section), ns, blocks, std::move(palette));
			palette = {};
			blocks = {};
			section = {};
			--sections_left;
		}
		else if (tag.isName("Y"))
			section.setY(tag.get<int8_t>());
		else if (tag.isName("BlockLight"))
			section.setBlockLight(tag.get<NBT::NBTByteArray>());
		else if (tag.isName("SkyLight"))
			section.setSkyLight(tag.get<NBT::NBTByteArray>());
		else if (tag.isName("Palette"))
		{
			palettes_left = tag.count();
			palette.reserve(std::size_t(palettes_left));
		}
		else if (tag.isName("BlockStates"))
		{
			auto & d = tag.get<NBT::NBTLongArray>();
			if (blocks.empty())
				blocks.resize(SECTION_SIZE);
			auto bits = d.size() / (SECTION_SIZE / 64);
			MC16::nibbleCopy(d, blocks, bits);
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
	else if (tag.isName("Heightmaps"))
		heightmaps = true;
	else if (tag.isName("Structures"))
		return true;
	else if (tag.isName("CarvingMasks"))
		return true;
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