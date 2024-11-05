#pragma once
#ifndef REGION_HPP
#define REGION_HPP

#include "vectorview.hpp"
#include "sharedfile.hpp"

#include <fstream>
#include <vector>
#include <map>
#include <memory>
#include <iterator>

namespace region
{

	using VectorData = VectorView<const uint8_t>;

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
			COMPRESSION_RAW, // Not official
			COMPRESSION_GZIP,
			COMPRESSION_ZLIB,
			COMPRESSION_UNCOMPRESSED,
			COMPRESSION_LZ4,
			COMPRESSION_CUSTOM
		};
		int32_t xPos, zPos;
		CompressionType compression_type = COMPRESSION_UNKNOWN;
		VectorData data;
	};

	class RegionChunk;

	/**
	 * Handles a specific region data
	 */
	class RegionFile : public SharedFile
	{
		struct Header
		{
			uint32_t i = 0;
			uint32_t offset : 24;
			uint32_t sector_count : 8;
			int32_t timestamp = 0;
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
			RegionFile * region;

		public:
			iterator() = default;
			iterator(const iterator & _it) = default;
			explicit iterator(Headers::iterator _it, RegionFile * _region);
			iterator& operator++();
			iterator operator++(int);
			bool operator==(const iterator & rhs) const;
			bool operator!=(const iterator & rhs) const;
			value_type operator*();
			pointer operator->();

		private:
			void ensureValidIterator();
		};

		RegionFile(int x, int z) noexcept;

		// File handling
		bool open(const std::string & path);
		bool openFile(const std::string & file);
		std::string file() const;
		void clear();

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
		std::string path;
		std::vector<std::shared_ptr<RegionChunk>> external_chunks;

		bool loadHeader();
		std::shared_ptr<ChunkData> getChunk(const Header & header);

		void preloadCache();
	};

	class RegionChunk : public SharedFile
	{
	public:
		RegionChunk(int x, int z, ChunkData::CompressionType compression) noexcept;

		// File handling
		bool open(const std::string & path);
		bool openFile(const std::string & file);
		std::string file() const;

		std::shared_ptr<ChunkData> getChunk();

		inline int x() const { return cx; }
		inline int z() const { return cz; }
	private:
		int cx, cz;
		ChunkData::CompressionType compression;
		std::vector<uint8_t> data;
	};

	/**
	* Handles all regions in a specific folder
	*/
	class Region
	{
		typedef std::map<std::pair<int, int>, std::shared_ptr<RegionFile>> RegionsMap;
	public:
		/**
		 * Iterates through all regions
		 */
		class iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::shared_ptr<RegionFile>;
			using difference_type = std::shared_ptr<RegionFile>;
			using pointer = RegionFile *;
			using reference = std::shared_ptr<RegionFile> &;
			RegionsMap::iterator it;
			Region & anvil;

		public:
			iterator() = delete;
			iterator(const iterator & _it) = default;
			explicit iterator(RegionsMap::iterator _it, Region & _anvil);
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
			Region::iterator region_it;
			Region::iterator region_end;
			RegionFile::iterator chunk_it;

		public:
			chunk_iterator() = delete;
			chunk_iterator(const chunk_iterator & _it) = default;
			explicit chunk_iterator(Region::iterator reg, Region::iterator end);
			chunk_iterator& operator++();
			chunk_iterator operator++(int);
			bool operator==(const chunk_iterator & rhs) const;
			bool operator!=(const chunk_iterator & rhs) const;
			value_type operator*();
			pointer operator->();

		private:
			void ensureValidIterator();
		};

		explicit Region(const std::string & path) noexcept;

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

} // namespace region

#endif // REGION_HPP
