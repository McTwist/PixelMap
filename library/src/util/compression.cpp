#include "util/compression.hpp"

#include "zlib.h"

// Note: Several power-of-two values have been tested, and this was the most fitting
constexpr uint32_t BUFFER_SIZE = 4096;

namespace Compression
{

std::vector<uint8_t> loadZLib(const std::vector<uint8_t> & compressed);
std::vector<uint8_t> loadZLibRaw(const std::vector<uint8_t> & compressed);
std::vector<uint8_t> loadGZip(const std::vector<uint8_t> & compressed);
std::vector<uint8_t> loadZLib(const VectorView<const uint8_t> & compressed);
std::vector<uint8_t> loadZLibRaw(const VectorView<const uint8_t> & compressed);
std::vector<uint8_t> loadGZip(const VectorView<const uint8_t> & compressed);

std::vector<uint8_t> loadCompressed(const std::vector<uint8_t> & compressed, int compression);
std::vector<uint8_t> loadCompressed(const VectorView<const uint8_t> & compressed, int compression);


// Load compressed data as zlib
std::vector<uint8_t> loadZLib(const std::vector<uint8_t> & compressed)
{
	return loadCompressed(compressed, MAX_WBITS);
}

// Load compressed data as zlib raw
std::vector<uint8_t> loadZLibRaw(const std::vector<uint8_t> & compressed)
{
	return loadCompressed(compressed, -MAX_WBITS);
}

// Load compressed data as gzip
std::vector<uint8_t> loadGZip(const std::vector<uint8_t> & compressed)
{
	return loadCompressed(compressed, 16 + MAX_WBITS);
}

// Load compressed data as zlib
std::vector<uint8_t> loadZLib(const VectorView<const uint8_t> & compressed)
{
	return loadCompressed(compressed, MAX_WBITS);
}

// Load compressed data as zlib raw
std::vector<uint8_t> loadZLibRaw(const VectorView<const uint8_t> & compressed)
{
	return loadCompressed(compressed, -MAX_WBITS);
}

// Load compressed data as gzip
std::vector<uint8_t> loadGZip(const VectorView<const uint8_t> & compressed)
{
	return loadCompressed(compressed, 16 + MAX_WBITS);
}

// Load compressed data with compression flag
std::vector<uint8_t> loadCompressed(const std::vector<uint8_t> & compressed, int compression)
{
	return loadCompressed(VectorView<const uint8_t>{compressed.data(), compressed.size()}, compression);
}

// Load compressed data with compression flag
std::vector<uint8_t> loadCompressed(const VectorView<const uint8_t> & compressed, int compression)
{
	if (compressed.empty())
		return {compressed.begin(), compressed.end()};
	// Prepare compression stream
	z_stream stream;

	std::vector<uint8_t> data;
	//data.reserve(compressed.size());

	Byte buffer[BUFFER_SIZE] = { 0 };

	stream.zalloc = nullptr;
	stream.zfree = nullptr;
	stream.opaque = nullptr;

	stream.next_in = const_cast<Byte *>(compressed.data());
	stream.avail_in = static_cast<uInt>(compressed.size());
	stream.next_out = buffer;
	stream.avail_out = BUFFER_SIZE;

	int ret = Z_OK;

	inflateInit2(&stream, compression);

	// Iterate through the stream
	do
	{
		stream.avail_out = BUFFER_SIZE;
		stream.next_out = buffer;
		ret = inflate(&stream, Z_NO_FLUSH);
		auto len = (stream.avail_out == 0) ? BUFFER_SIZE : BUFFER_SIZE - stream.avail_out;
		data.insert(data.end(), buffer, buffer + len);
	}
	while (stream.avail_out == 0 && ret != Z_STREAM_END);

	inflateEnd(&stream);

	return data;
}

} // namespace Compression
