#ifndef BEDROCK_WORLD_HPP
#define BEDROCK_WORLD_HPP

#include "chunk.hpp"
#include "render/utility.hpp"
#include "lightsource.hpp"

#include <cstdint>
#include <unordered_map>

class ChunkRender;
struct ChunkRenderData;

namespace bedrock
{

class World
{
public:
	World(const std::string & name, int32_t dimension);
	Chunk & getChunk(int32_t x, int32_t y);
	void setSection(int32_t x, int32_t y, const SectionData & section);
	void setHeightmap(int32_t x, int32_t y, const NBT::NBTIntArray & heightMap);
	void generateBlockLight(const LightSource & lightsource);
	
	void draw(const std::function<std::shared_ptr<ChunkRenderData>(const Chunk &)> & render);
	void merge(const World & world);

	bool operator<(const World & w) const { return name < w.name; }

	int32_t getDimension() const { return dimension; }

	std::size_t size() const { return chunks.size(); }

private:
	std::string name;
	int32_t dimension;
	std::unordered_map<utility::ChunkPosition, Chunk> chunks;
	std::unordered_map<utility::ChunkPosition, std::shared_ptr<ChunkRenderData>> renderData;

public:

	class iterator
	{
		using iterator_category = std::forward_iterator_tag;
		using value_type = Chunk;
		using difference_type = Chunk;
		using pointer = Chunk *;
		using reference = Chunk &;
		decltype(chunks)::iterator it;

	public:
		iterator() = delete;
		iterator(const iterator & _it) = default;
		explicit iterator(const decltype(it) & _it) : it(_it) {}
		iterator& operator++() { ++it; return *this; }
		iterator operator++(int) { iterator tmp(*this); operator++(); return tmp; }
		bool operator==(const iterator & rhs) const { return it == rhs.it; }
		bool operator!=(const iterator & rhs) const { return it != rhs.it; }
		reference operator*() { return it->second; }
		pointer operator->() { return &it->second; }
	};

	iterator begin() { return iterator(chunks.begin()); }
	iterator end() { return iterator(chunks.end()); }

	const decltype(renderData) & render() const { return renderData; }
};

} // namespace bedrock

#endif // BEDROCK_WORLD_HPP
