#include "util/palette.hpp"

#include "anvil/limits.hpp"

void palette::translate(
		Chunk & chunk,
		SectionData && section,
		std::array<uint16_t, ID_SIZE> & id,
		std::vector<uint16_t> & blocks)
{
	uint16_t idx = uint16_t(chunk.getIDPalette().size());
	// Translate all blocks to a palette
	for (auto i = 0U; i < blocks.size(); ++i)
	{
		auto & block = blocks[i];
		auto it = id[block];
		// Add new
		if (it == BLOCK_ID_MAX)
		{
			chunk.addPalette(block);
			id[block] = idx;
			block = idx;
			++idx;
		}
		// Add old
		else
		{
			block = it;
		}
	}
	section.setBlocks(blocks);
	chunk.setSection(std::move(section));
}

void palette::translate(
		Chunk & chunk,
		SectionData && section,
		std::unordered_map<std::string, uint16_t> & ns,
		std::vector<uint16_t> & blocks,
		std::vector<std::string> & palette)
{
	uint16_t idx = uint16_t(chunk.getNSPalette().size());

	// Prepare translation table for palette
	std::vector<uint16_t> translation(palette.size());
	for (auto i = 0U; i < palette.size(); ++i)
	{
		auto & name = palette[i];
		auto it	= ns.find(name);
		// Add new
		if (it == ns.end())
		{
			chunk.addPalette(name);
			ns[name] = idx;
			translation[i] = idx;
			++idx;
		}
		// Add old
		else
		{
			translation[i] = it->second;
		}
	}
	// Translate all blocks to a palette
	for (auto i = 0U; i < blocks.size(); ++i)
	{
		auto & block = blocks[i];
		block = translation[block];
	}
	section.setBlocks(blocks);
	chunk.setSection(std::move(section));
}
