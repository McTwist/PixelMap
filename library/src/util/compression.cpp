#include "util/compression.hpp"

#ifdef USE_LIBDEFLATE
#include "libdeflate.h"
#include <functional>
#else
#include "zlib.h"
#endif

#include "lz4.h"

// Note: Several power-of-two values have been tested, and this was the most fitting
#ifdef USE_LIBDEFLATE
constexpr uint32_t DEFLATE_BUFFER_SIZE = 65536;
#else
constexpr uint32_t DEFLATE_BUFFER_SIZE = 4096;
#endif

// TODO: Test more values
constexpr uint32_t LZ4_BUFFER_SIZE = 65536;

namespace Compression
{

#ifdef USE_LIBDEFLATE

using decompress = decltype(libdeflate_deflate_decompress);

inline std::vector<uint8_t> loadCompressed(const std::vector<uint8_t> & compressed, std::function<decompress> func);
static std::vector<uint8_t> loadCompressed(const VectorView<const uint8_t> & compressed, std::function<decompress> func);

#define LOAD_ZLIB libdeflate_zlib_decompress
#define LOAD_DEFLATE libdeflate_deflate_decompress
#define LOAD_GZIP libdeflate_gzip_decompress

#else

inline std::vector<uint8_t> loadCompressed(const std::vector<uint8_t> & compressed, int compression);
static std::vector<uint8_t> loadCompressed(const VectorView<const uint8_t> & compressed, int compression);

#define LOAD_ZLIB MAX_WBITS
#define LOAD_DEFLATE -MAX_WBITS
#define LOAD_GZIP 16 + MAX_WBITS

#endif // USE_LIBDEFLATE

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

#ifdef USE_LIBDEFLATE

inline std::vector<uint8_t> loadCompressed(const std::vector<uint8_t> & compressed, std::function<decompress> func)
{
	return loadCompressed(VectorView<const uint8_t>{compressed.data(), compressed.size()}, func);
}

static std::vector<uint8_t> loadCompressed(const VectorView<const uint8_t> & compressed, std::function<decompress> func)
{
	if (compressed.empty())
		return {compressed.begin(), compressed.end()};
	// Prepare compression stream
	libdeflate_decompressor * stream;

	std::vector<uint8_t> data;
	data.resize(DEFLATE_BUFFER_SIZE);

	const void * in = compressed.data();
	size_t in_nbytes = compressed.size();
	size_t actual_out_nbytes_ret = 0;

	stream = libdeflate_alloc_decompressor();

	auto ret = LIBDEFLATE_SUCCESS;

	// Try larger and larger size of output buffer
	do
	{
		void * out = data.data();
		size_t out_nbytes_avail = data.size();
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
inline std::vector<uint8_t> loadCompressed(const std::vector<uint8_t> & compressed, int compression)
{
	return loadCompressed(VectorView<const uint8_t>{compressed.data(), compressed.size()}, compression);
}

// Load compressed data with compression flag
static std::vector<uint8_t> loadCompressed(const VectorView<const uint8_t> & compressed, int compression)
{
	if (compressed.empty())
		return {compressed.begin(), compressed.end()};
	// Prepare compression stream
	z_stream stream;

	std::vector<uint8_t> data;
	//data.reserve(compressed.size());

	Byte buffer[DEFLATE_BUFFER_SIZE] = { 0 };

	stream.zalloc = nullptr;
	stream.zfree = nullptr;
	stream.opaque = nullptr;

	stream.next_in = const_cast<Byte *>(compressed.data());
	stream.avail_in = static_cast<uInt>(compressed.size());
	stream.next_out = buffer;
	stream.avail_out = DEFLATE_BUFFER_SIZE;

	int ret = Z_OK;

	inflateInit2(&stream, compression);

	// Iterate through the stream
	do
	{
		stream.avail_out = DEFLATE_BUFFER_SIZE;
		stream.next_out = buffer;
		ret = inflate(&stream, Z_NO_FLUSH);
		auto len = (stream.avail_out == 0) ? DEFLATE_BUFFER_SIZE : DEFLATE_BUFFER_SIZE - stream.avail_out;
		data.insert(data.end(), buffer, buffer + len);
	}
	while (stream.avail_out == 0 && ret != Z_STREAM_END);

	inflateEnd(&stream);

	return data;
}

#endif // USE_LIBDEFLATE

std::vector<uint8_t> loadLZ4(const std::vector<uint8_t> & compressed)
{
	return loadLZ4(VectorView<const uint8_t>{compressed.data(), compressed.size()});
}
std::vector<uint8_t> loadLZ4(const VectorView<const uint8_t> & compressed)
{
	if (compressed.empty())
		return {compressed.begin(), compressed.end()};

	std::vector<uint8_t> data;
	data.resize(LZ4_BUFFER_SIZE);

	const void * src = compressed.data();
	size_t compressedSize = compressed.size();

	auto ret = 0;

	// Try larger and larger size of output buffer
	do
	{
		void * dst = data.data();
		size_t dstCapacity = data.size();
		ret = LZ4_decompress_safe(
			reinterpret_cast<const char *>(src),
			reinterpret_cast<char *>(dst),
			compressedSize, dstCapacity);
		if (ret < 0)
			data.resize(data.size() << 1);
	}
	while (ret < 0);

	data.resize(ret);
	data.shrink_to_fit();

	return data;
}

} // namespace Compression
