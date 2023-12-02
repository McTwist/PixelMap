#pragma once
#ifndef ANVIL_HPP
#define ANVIL_HPP

#include "vectorview.hpp"
#include "sharedfile.hpp"

#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <iterator>

namespace anvil
{

	typedef VectorView<const uint8_t> VectorData;

	/**
	 * Data for a chunk to be used outside
	 */
	struct ChunkData
	{
		ChunkData(int32_t xPos, int32_t zPos)
			: xPos(xPos), zPos(zPos)
		{}
		ChunkData(const ChunkData &) = default;
		ChunkData(ChunkData &&) = default;

		enum CompressionType: int8_t
		{
			COMPRESSION_UNKNOWN = -1,
			COMPRESSION_RAW,
			COMPRESSION_GZIP,
			COMPRESSION_ZLIB
		};
		int32_t xPos, zPos;
		CompressionType compression_type = COMPRESSION_UNKNOWN;
		VectorData data;
	};

	/**
	 * Handles a specific region data
	 */
	class AnvilRegion : public SharedFile
	{
		struct Header
		{
			uint32_t i = 0;
			uint32_t offset : 24;
			uint32_t sector_count : 8;
			int timestamp = 0;
		};
		typedef std::vector<Header> Headers;
	public:
		/**
		 * Iterates through all chunks
		 */
		class iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::shared_ptr<ChunkData>;
			using difference_type = std::shared_ptr<ChunkData>;
			using pointer = ChunkData *;
			using reference = std::shared_ptr<ChunkData> &;
			Headers::iterator it;
			AnvilRegion * region;

		public:
			iterator() = default;
			iterator(const iterator & _it) = default;
			explicit iterator(Headers::iterator _it, AnvilRegion * _region);
			iterator& operator++();
			iterator operator++(int);
			bool operator==(const iterator & rhs) const;
			bool operator!=(const iterator & rhs) const;
			value_type operator*();
			pointer operator->();

		private:
			void ensureValidIterator();
		};

		AnvilRegion(int x, int z) noexcept;

		// File handling
		bool open(const std::string & path);
		bool openFile(const std::string & file);
		std::string file() const;

		// Get timestamp of chunk
		int getChunkTimestamp(int x, int z);
		// Get a chunk from position
		std::shared_ptr<ChunkData> getChunk(int x, int z);
		bool containsChunk(int x, int z) const;

		// Iterate through each chunk
		iterator begin();
		iterator end();

		inline int x() const { return rx; }
		inline int z() const { return rz; }
		inline int getAmountChunks() const { return amount_chunks; }

	private:
		int rx, rz;
		int amount_chunks = 0;
		Headers headers;
		std::vector<uint8_t> cache;

		std::shared_ptr<ChunkData> getChunk(const Header & header);

		void preloadCache();
	};

	/**
	* Handles all regions in a specific folder
	*/
	class Anvil
	{
		typedef std::map<std::pair<int, int>, std::shared_ptr<AnvilRegion>> RegionsMap;
	public:
		/**
		 * Iterates through all regions
		 */
		class iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::shared_ptr<AnvilRegion>;
			using difference_type = std::shared_ptr<AnvilRegion>;
			using pointer = AnvilRegion *;
			using reference = std::shared_ptr<AnvilRegion> &;
			RegionsMap::iterator it;
			Anvil & anvil;

		public:
			iterator() = delete;
			iterator(const iterator & _it) = default;
			explicit iterator(RegionsMap::iterator _it, Anvil & _anvil);
			iterator & operator++();
			iterator operator++(int);
			bool operator==(const iterator & rhs) const;
			bool operator!=(const iterator & rhs) const;
			reference operator*();
			pointer operator->();
		};

		/**
		 * Iterates through all chunks on all regions
		 */
		class chunk_iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::shared_ptr<ChunkData>;
			using difference_type = std::shared_ptr<ChunkData>;
			using pointer = ChunkData *;
			using reference = std::shared_ptr<ChunkData> &;
			Anvil::iterator region_it;
			Anvil::iterator region_end;
			AnvilRegion::iterator chunk_it;

		public:
			chunk_iterator() = delete;
			chunk_iterator(const chunk_iterator & _it) = default;
			explicit chunk_iterator(Anvil::iterator reg, Anvil::iterator end);
			chunk_iterator& operator++();
			chunk_iterator operator++(int);
			bool operator==(const chunk_iterator & rhs) const;
			bool operator!=(const chunk_iterator & rhs) const;
			value_type operator*();
			pointer operator->();

		private:
			void ensureValidIterator();
		};

		explicit Anvil(const std::string & path) noexcept;

		// Get timestamp of chunk
		int getChunkTimestamp(int x, int z);
		// Get chunk from coordinates
		std::shared_ptr<ChunkData> getChunk(int x, int z);

		// Iterate through each region
		// Note: This is recommended for scalable applications
		iterator begin();
		iterator end();

		// Iterate through each chunk
		chunk_iterator beginChunk();
		chunk_iterator endChunk();

		// Checks if filename of file is valid
		static bool validateFileName(const std::string & file);

	private:
		std::string path;
		RegionsMap regions;
		RegionsMap::iterator region_it;

		friend iterator;

		void populateFromPath();
	};

} // namespace anvil

#endif // ANVIL_HPP
