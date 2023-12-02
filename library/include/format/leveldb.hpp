#pragma once
#ifndef LEVELDB_HPP
#define LEVELDB_HPP

#include "vectorview.hpp"
#include "sharedfile.hpp"

#include <fstream>
#include <vector>
#include <string>
#include <memory>
#include <stdint.h>
#include <iterator>
#include <functional>
#include <unordered_map>

namespace LevelDB
{

	typedef VectorView<const uint8_t> VectorData;

	struct ChunkData
	{
		ChunkData() = default;
		ChunkData(int32_t xPos, int32_t zPos)
			: xPos(xPos), zPos(zPos)
		{}
		ChunkData(const ChunkData &) = default;
		ChunkData(ChunkData &&) = default;

		struct ChunkSubData {
			int32_t yPos;
			VectorData blocks;
		};

		int32_t xPos, zPos;
		std::vector<int16_t> heightmap;
		std::unordered_map<int8_t, std::vector<uint16_t>> sections;
		std::vector<std::string> palette;
	};

	class LevelFile : public SharedFile
	{
	public:

		LevelFile(const std::string & file) noexcept;

		// File handling
		bool open(const std::string & path);
		std::string file() const { return _file; }

	private:
		std::string _file;
	};

	class LevelDB
	{
		typedef std::vector<std::shared_ptr<LevelFile>> Levels;
	public:
		/**
		 * Iterates through all regions
		 */
		class iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::shared_ptr<LevelFile>;
			using difference_type = std::shared_ptr<LevelFile>;
			using pointer = LevelFile *;
			using reference = std::shared_ptr<LevelFile> &;
			Levels::iterator it;
			LevelDB & leveldb;

		public:
			iterator() = delete;
			iterator(const iterator & _it) = default;
			explicit iterator(Levels::iterator _it, LevelDB & _leveldb);
			iterator & operator++();
			iterator operator++(int);
			bool operator==(const iterator & rhs) const;
			bool operator!=(const iterator & rhs) const;
			reference operator*();
			pointer operator->();
		};

		explicit LevelDB(const std::string & path) noexcept;

		// Iterate through each file
		// Note: This is recommended for scalable applications
		iterator begin();
		iterator end();

		std::size_t size() const;

		std::shared_ptr<LevelFile> getLog() const;

	private:
		std::string path;
		Levels levels;
		std::string log;

		void populateFromPath();
	};

	class Visitor
	{
	protected:
		virtual ~Visitor() = default;
	public:
		virtual void visit(const std::vector<uint8_t> & key, const VectorData & data) = 0;
	};

	class Reader
	{
	public:
		// Parse the data with a visitor
		std::ptrdiff_t parse(std::vector<uint8_t> & data, Visitor & visitor);
		std::ptrdiff_t parse(VectorView<uint8_t> data, Visitor & visitor);

		// Parse the data with a pair of functions
		std::ptrdiff_t parse(std::vector<uint8_t> & data, std::function<void(const std::vector<uint8_t> &, const VectorData &)> visit);
		virtual std::ptrdiff_t parse(VectorView<uint8_t> data, std::function<void(const std::vector<uint8_t> &, const VectorData &)> visit) = 0;

		// Get previous error as a string
		const std::string & getError() const { return error; }

	private:
		std::string error;

	protected:
		// Throw an error
		std::ptrdiff_t throwError(const std::string & err);
	};

	class LevelReader : public Reader
	{
	public:
		using Reader::parse;
		std::ptrdiff_t parse(VectorView<uint8_t> data, std::function<void(const std::vector<uint8_t> &, const VectorData &)> visit);
	};

	class LogReader : public Reader
	{
	public:
		using Reader::parse;
		std::ptrdiff_t parse(VectorView<uint8_t> data, std::function<void(const std::vector<uint8_t> &, const VectorData &)> visit);
	};

} // namespace LevelDB

#endif // LEVELDB_HPP
