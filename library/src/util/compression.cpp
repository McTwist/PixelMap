#include "util/compression.hpp"

#define USE_DEFLATE

#ifdef USE_DEFLATE
#include "libdeflate.h"
#include <functional>
#else
#include "zlib.h"
#endif

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

#ifdef USE_DEFLATE

using decompress = decltype(libdeflate_deflate_decompress);

std::vector<uint8_t> loadCompressed(const std::vector<uint8_t> & compressed, std::function<decompress> func);
std::vector<uint8_t> loadCompressed(const VectorView<const uint8_t> & compressed, std::function<decompress> func);

#define LOAD_ZLIB libdeflate_zlib_decompress
#define LOAD_DEFLATE libdeflate_deflate_decompress
#define LOAD_GZIP libdeflate_gzip_decompress

#else

std::vector<uint8_t> loadCompressed(const std::vector<uint8_t> & compressed, int compression);
std::vector<uint8_t> loadCompressed(const VectorView<const uint8_t> & compressed, int compression);

#define LOAD_ZLIB MAX_WBITS
#define LOAD_DEFLATE -MAX_WBITS
#define LOAD_GZIP 16 + MAX_WBITS

#endif // USE_DEFLATE

// Load compressed data as zlib
std::vector<uint8_t> loadZLib(const std::vector<uint8_t> & compressed)
{
	return loadCompressed(compressed, LOAD_ZLIB);
}

// Load compressed data as zlib raw
std::vector<uint8_t> loadZLibRaw(const std::vector<uint8_t> & compressed)
{
	return loadCompressed(compressed, LOAD_DEFLATE);
}

// Load compressed data as gzip
std::vector<uint8_t> loadGZip(const std::vector<uint8_t> & compressed)
{
	return loadCompressed(compressed, LOAD_GZIP);
}

// Load compressed data as zlib
std::vector<uint8_t> loadZLib(const VectorView<const uint8_t> & compressed)
{
	return loadCompressed(compressed, LOAD_ZLIB);
}

// Load compressed data as zlib raw
std::vector<uint8_t> loadZLibRaw(const VectorView<const uint8_t> & compressed)
{
	return loadCompressed(compressed, LOAD_DEFLATE);
}

// Load compressed data as gzip
std::vector<uint8_t> loadGZip(const VectorView<const uint8_t> & compressed)
{
	return loadCompressed(compressed, LOAD_GZIP);
}

#ifdef USE_DEFLATE

std::vector<uint8_t> loadCompressed(const std::vector<uint8_t> & compressed, std::function<decompress> func)
{
	return loadCompressed(VectorView<const uint8_t>{compressed.data(), compressed.size()}, func);
}

std::vector<uint8_t> loadCompressed(const VectorView<const uint8_t> & compressed, std::function<decompress> func)
{
	if (compressed.empty())
		return {compressed.begin(), compressed.end()};
	// Prepare compression stream
	libdeflate_decompressor * stream;

	std::vector<uint8_t> data;
	data.resize(BUFFER_SIZE * 16);

	const void * in = compressed.data();
	size_t in_nbytes = compressed.size();
	void * out = data.data();
	size_t out_nbytes_avail = data.size();
	size_t actual_out_nbytes_ret = 0;

	stream = libdeflate_alloc_decompressor();

	auto ret = LIBDEFLATE_SUCCESS;

	// Try larger and larger size of output buffer
	do
	{
		out = data.data();
		out_nbytes_avail = data.size();
		ret = func(stream, in, in_nbytes, out, out_nbytes_avail, &actual_out_nbytes_ret);
		if (ret == LIBDEFLATE_INSUFFICIENT_SPACE)
			data.resize(data.size() << 1);
	}
	while (ret != LIBDEFLATE_SUCCESS);

	libdeflate_free_decompressor(stream);

	data.resize(actual_out_nbytes_ret);
	data.shrink_to_fit();

	return data;
}

#else

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

#endif // USE_DEFLATE

} // namespace Compression
