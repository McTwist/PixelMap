#include "catch2/catch.hpp"

#include "format/varint.hpp"

#include <limits>

TEST_CASE("varint", "[format]")
{
	SECTION("8-bit")
	{
		SECTION("zero")
		{
			const uint8_t _test[] = {0b00000000, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int8_t>(_ptr) == 0);
			REQUIRE(_ptr == _test+1);
		}
		SECTION("one")
		{
			const uint8_t _test[] = {0b00000001, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int8_t>(_ptr) == 1);
			REQUIRE(_ptr == _test+1);
			REQUIRE(*_ptr == 0);
		}
		SECTION("max")
		{
			const uint8_t _test[] = {0b01111111, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int8_t>(_ptr) == std::numeric_limits<int8_t>::max());
			REQUIRE(_ptr == _test+1);
			REQUIRE(*_ptr == 0);
		}
		SECTION("min")
		{
			const uint8_t _test[] = {0b10000000, 0b00000001, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int8_t>(_ptr) == std::numeric_limits<int8_t>::min());
			REQUIRE(_ptr == _test+2);
			REQUIRE(*_ptr == 0);
		}
	}
	SECTION("16-bit")
	{
		SECTION("300")
		{
			const uint8_t _test[] = {0b10101100, 0b00000010, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int16_t>(_ptr) == 300);
			REQUIRE(_ptr == _test+2);
			REQUIRE(*_ptr == 0);
		}
		SECTION("max")
		{
			const uint8_t _test[] = {0b11111111, 0b11111111, 0b01000001, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int16_t>(_ptr) == std::numeric_limits<int16_t>::max());
			REQUIRE(_ptr == _test+3);
			REQUIRE(*_ptr == 0);
		}
		SECTION("min")
		{
			const uint8_t _test[] = {0b10000000, 0b10000000, 0b00000010, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int16_t>(_ptr) == std::numeric_limits<int16_t>::min());
			REQUIRE(_ptr == _test+3);
			REQUIRE(*_ptr == 0);
		}
	}
	SECTION("32-bit")
	{
		SECTION("max")
		{
			const uint8_t _test[] = {0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b00000111, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int32_t>(_ptr) == std::numeric_limits<int32_t>::max());
			REQUIRE(_ptr == _test+5);
			REQUIRE(*_ptr == 0);
		}
		SECTION("min")
		{
			const uint8_t _test[] = {0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b00001000, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int32_t>(_ptr) == std::numeric_limits<int32_t>::min());
			REQUIRE(_ptr == _test+5);
			REQUIRE(*_ptr == 0);
		}
	}
	SECTION("64-bit")
	{
		SECTION("max")
		{
			const uint8_t _test[] = {
				0b11111111, 0b11111111, 0b11111111, 0b11111111,
				0b11111111, 0b11111111, 0b11111111, 0b11111111,
				0b01111111, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int64_t>(_ptr) == std::numeric_limits<int64_t>::max());
			REQUIRE(_ptr == _test+9);
			REQUIRE(*_ptr == 0);
		}
		SECTION("min")
		{
			const uint8_t _test[] = {
				0b10000000, 0b10000000, 0b10000000, 0b10000000,
				0b10000000, 0b10000000, 0b10000000, 0b10000000,
				0b10000000, 0b00000001, 0};
			auto _ptr = _test;
			CHECK(leveldb::read_varint<int64_t>(_ptr) == std::numeric_limits<int64_t>::min());
			REQUIRE(_ptr == _test+10);
			REQUIRE(*_ptr == 0);
		}
	}
	SECTION("skip")
	{
		const uint8_t _test[] = {
			0b11111111, 0b11111111, 0b11111111, 0b11111111,
			0b11111111, 0b11111111, 0b11111111, 0b11111111,
			0b01111111, 0};
		auto _ptr = _test;
		leveldb::skip_varint(_ptr);
		REQUIRE(*_ptr == 0);
	}
	SECTION("dummy")
	{
		const uint8_t _test[] = {
			0b10000000, 0b10000000, 0b10000000, 0b10000000,
			0b10000000, 0b10000000, 0b10000000, 0b10000000,
			0b10000000, 0b10000000, 0b10000000, 0b00000000, 0};
		auto _ptr = _test;
		CHECK(leveldb::read_varint<int64_t>(_ptr) == 0);
		REQUIRE(*_ptr == 0);
	}
}

TEST_CASE("nibble")
{
	uint8_t bits = GENERATE(1, 2, 3, 4, 5, 6, 8, 10, 14, 16, 32);
    // Minecraft
    auto blocksPerWord = std::floor(32.0 / bits);
    auto wordCount = std::ceil(4096.0 / blocksPerWord);
    // Integer
    auto parts = 32 / bits;
	auto size = (4095 + parts) / parts;
	// Result
    REQUIRE(wordCount == size);
}

