#include "format/alpha.hpp"

#include "platform.hpp"
#include "string.hpp"

#include <fmt/format.h>

#include <regex>
#include <cmath>
#include <filesystem>
#include <stdexcept>

namespace base36
{

static const std::regex reg{"^[0-9a-z]+$"};

static bool is(const std::string & str)
{
	return std::regex_match(str, reg);
}

static int from(const std::string & str)
{
	int ret = 0;
	for (std::size_t i = 0; i < str.size(); ++i)
	{
		auto c = str[(str.size() - 1) - i];
		auto p = std::pow(36, i);
		if ('0' <= c && c <= '9')
			ret += (c - '0') * p;
		else if ('a' <= c && c <= 'z')
			ret += (10 + c - 'a') * p;
		else if ('A' <= c && c <= 'Z')
			ret += (10 + c - 'A') * p;
		else
			throw std::invalid_argument(fmt::format("Invalid base36 '{}'", c));
	}
	return ret;
}

}

namespace alpha
{

Alpha::Alpha(const std::string & path) noexcept
	: path(path)
{
}

Alpha::iterator Alpha::begin()
{
	// Get all files
	populateFromPath();

	return iterator(files.begin(), *this);
}

Alpha::iterator Alpha::end()
{
	return iterator(files.end(), *this);
}

void Alpha::populateFromPath()
{
	if (!std::filesystem::is_directory(path))
		return;

	files.clear();

	std::regex r{"^c\\.(-?[0-9]+)\\.(-?[0-9]+)\\.dat$"};
	std::error_code ec;
	std::string name;
	for (auto & e1 : std::filesystem::directory_iterator{path, ec})
	{
		if (!e1.is_directory())
			continue;
		name = e1.path().filename().string();
		if (!base36::is(name))
			continue;
		for (auto & e2 : std::filesystem::directory_iterator{e1.path(), ec})
		{
			if (!e2.is_directory())
				continue;
			name = e2.path().filename().string();
			if (!base36::is(name))
				continue;
			for (auto & e3 : std::filesystem::directory_iterator{e2.path(), ec})
			{
				if (e3.is_directory())
					continue;
				name = e3.path().filename().string();
				std::smatch m;
				if (!std::regex_match(name, m, r))
					continue;
				if (m.size() != 3)
					continue;

				int x, z;

				try
				{
					auto sx = m[1].str();
					if (sx[0] == '-')
						x = -base36::from(sx.substr(1));
					else
						x = base36::from(sx);
					auto sz = m[2].str();
					if (sz[0] == '-')
						z = -base36::from(sz.substr(1));
					else
						z = base36::from(sz);
				}
				catch(...)
				{
					continue;
				}

				files.insert(std::make_pair(std::make_pair(x, z), std::make_shared<AlphaFile>(x, z, e3.path().string())));
			}
		}
	}
}


AlphaFile::AlphaFile(int x, int z, const std::string & path) noexcept
	: cx(x), cz(z), path(path)
{
}

bool AlphaFile::open()
{
	return openFile(path);
}

bool AlphaFile::open(const std::string & _path)
{
	path = _path;
	return openFile(platform::path::join(path, file()));
}

bool AlphaFile::openFile(const std::string & file)
{
	if (!SharedFile::openFile(file))
		return false;
	return true;
}

std::string AlphaFile::file() const
{
	return string::format("c.", cx, ".", cz, ".dat");
}


/*
 * Iterators
 */

Alpha::iterator::iterator(AlphasMap::iterator _it, Alpha & _alpha) :
	it(_it),
	alpha(_alpha)
{
}
Alpha::iterator::iterator(const iterator & _it) :
	it(_it.it),
	alpha(_it.alpha)
{
}
Alpha::iterator & Alpha::iterator::operator++()
{
	++it;
	return *this;
}
Alpha::iterator Alpha::iterator::operator++(int)
{
	iterator tmp(*this);
	operator++();
	return tmp;
}
bool Alpha::iterator::operator==(const iterator & rhs) const
{
	return it == rhs.it;
}
bool Alpha::iterator::operator!=(const iterator & rhs) const
{
	return it != rhs.it;
}
Alpha::iterator::value_type Alpha::iterator::operator*()
{
	return it->second;
}
Alpha::iterator::pointer Alpha::iterator::operator->()
{
	// Go around pointer issue
	return it->second.get();
}

}
