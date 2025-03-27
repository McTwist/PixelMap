#pragma once
#ifndef ALPHA_HPP
#define ALPHA_HPP

#include "sharedfile.hpp"

#include <map>
#include <memory>
#include <iterator>

namespace alpha
{

	class AlphaFile : public SharedFile
	{
	public:
		AlphaFile(int x, int z, const std::string & path) noexcept;

		// File handling
		bool open();
		bool open(const std::string & path);
		bool openFile(const std::string & file);
		std::string file() const;

		inline int x() const { return cx; }
		inline int z() const { return cz; }
	private:
		int cx, cz;
		std::string path;
	};

	class Alpha
	{
		typedef std::map<std::pair<int, int>, std::shared_ptr<AlphaFile>> AlphasMap;
	public:
		/**
		 * Iterates through all chunks
		 */
		class iterator
		{
			using iterator_category = std::forward_iterator_tag;
			using value_type = std::shared_ptr<AlphaFile>;
			using difference_type = std::shared_ptr<AlphaFile>;
			using pointer = AlphaFile *;
			using reference = std::shared_ptr<AlphaFile> &;
			AlphasMap::iterator it;
			Alpha & alpha;

		public:
			iterator() = delete;
			iterator(const iterator & _it) = default;
			explicit iterator(AlphasMap::iterator _it, Alpha & _alpha);
			iterator& operator++();
			iterator operator++(int);
			bool operator==(const iterator & rhs) const;
			bool operator!=(const iterator & rhs) const;
			value_type operator*();
			pointer operator->();
		};

		explicit Alpha(const std::string & path) noexcept;

		// Iterate through each chunk
		iterator begin();
		iterator end();

		std::size_t count() const { return files.size(); }

	private:
		std::string path;
		AlphasMap files;

		friend iterator;

		void populateFromPath();
	};

}

#endif // ALPHA_HPP
