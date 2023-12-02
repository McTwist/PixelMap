#include "sharedfile.hpp"

#include "platform.hpp"

#include <spdlog/spdlog.h>

SharedFile::SharedFile()
{
	_in = std::make_shared<std::ifstream>();
}

SharedFile::~SharedFile()
{
	close();
}

bool SharedFile::openFile(const std::string & file)
{
	if (_in->is_open())
	{
		_last_error = "File is already open";
		return false;
	}
	platform::fd::enter();

	_in->open(file.c_str(), std::ios::in | std::ios::binary);
	if (!_in->is_open())
	{
		platform::fd::leave();
		_last_error = "Failed to open file";
		return false;
	}

	return true;
}

void SharedFile::close()
{
	if (_in.use_count() == 1 && _in->is_open())
	{
		_in->close();
		platform::fd::leave();
	}
}

std::vector<uint8_t> SharedFile::readAll()
{
	if (!_in->is_open())
	{
		_last_error = "File is not open";
		return {};
	}
	std::vector<uint8_t> data;
	_in->seekg(0, std::ios::end);
	auto size = _in->tellg();
	_in->seekg(0, std::ios::beg);
	data.resize(size);
	read(data.data(), size);
	return data;
}

inline bool SharedFile::read(uint8_t * ptr, std::size_t size)
{
	_in->read(reinterpret_cast<char *>(ptr), std::streamsize(size));
	if (std::size_t(_in->gcount()) != size)
	{
		_last_error = "Invalid read from file";
		return false;
	}
	return true;
}

bool SharedFile::isOpen() const
{
	return _in->is_open();
}

uint64_t SharedFile::size() const
{
	_in->seekg(0, std::ios::end);
	auto _size = _in->tellg();
	_in->seekg(0, std::ios::beg);
	return _size;
}

void SharedFile::seek(uint64_t offset)
{
	_in->seekg(offset, std::ios::beg);
}

