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
	_file = file;

	_in->open(file.c_str(), std::ios::in | std::ios::binary);
	if (!_in->is_open())
	{
		_last_error = "Failed to open file";
		return false;
	}

	// Store for later use
	_in->seekg(0, std::ios::end);
	_size = _in->tellg();
	_in->seekg(0, std::ios::beg);

	return true;
}

void SharedFile::close()
{
	if (_in.use_count() == 1 && _in->is_open())
	{
		_in->close();
	}
}

std::vector<uint8_t> SharedFile::readAll()
{
	if (!ensureOpen())
		return {};
	std::vector<uint8_t> data;
	_in->seekg(0, std::ios::end);
	auto size = _in->tellg();
	_in->seekg(0, std::ios::beg);
	data.resize(size);
	read(data.data(), size);
	return data;
}

bool SharedFile::read(uint8_t * ptr, std::size_t size)
{
	if (!ensureOpen())
		return false;
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
	return _in->is_open() || !_file.empty();
}

uint64_t SharedFile::size() const
{
	return _size;
}

void SharedFile::seek(uint64_t offset)
{
	ensureOpen();
	_in->seekg(offset, std::ios::beg);
}

inline bool SharedFile::ensureOpen()
{
	if (_in->is_open())
		return true;
	if (!_file.empty())
		return openFile(_file);
	_last_error = "File is not open";
	return false;
}
