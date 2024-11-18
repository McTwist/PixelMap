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
		std::vector<std::string> && palette)
{
	uint16_t idx = uint16_t(chunk.getNSPalette().size());

	// Prepare translation table for palette
	struct PaletteTranslation
	{
		std::string name;
		uint16_t translation = BLOCK_ID_MAX;
	};
	std::vector<PaletteTranslation> translation(palette.size());
	for (auto i = 0U; i < palette.size(); ++i)
	{
		auto name = std::move(palette[i]);
		auto it	= ns.find(name);
		// Add new
		if (it == ns.end())
		{
			translation[i].name = std::move(name);
		}
		// Add old
		else
		{
			translation[i] = {std::move(name), it->second};
		}
	}
	// Translate all blocks to a palette
	for (auto i = 0U; i < blocks.size(); ++i)
	{
		auto & block = blocks[i];
		auto & translate = translation[block];
		// Only add palette names that actually is used
		if (translate.translation == BLOCK_ID_MAX)
		{
			translate.translation = idx;
			ns[translate.name] = idx;
			chunk.addPalette(std::move(translate.name));
			++idx;
		}
		block = translate.translation;
	}
	palette.clear();
	section.setBlocks(blocks);
	chunk.setSection(std::move(section));
}
