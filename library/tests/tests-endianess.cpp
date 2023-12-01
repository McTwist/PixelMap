#include "catch2/catch.hpp"

#include "util/endianess.hpp"

#include <vector>
#include <limits>

using Catch::Matchers::Equals;

TEST_CASE("endianess", "[utility]")
{
	SECTION("toLittle")
	{
		SECTION("short max")
		{
			std::vector<uint8_t> _out(2), _verify = {255, 127};
			endianess::toLittle(std::numeric_limits<int16_t>::max(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
		SECTION("short min")
		{
			std::vector<uint8_t> _out(2), _verify = {0, 128};
			endianess::toLittle(std::numeric_limits<int16_t>::min(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
		SECTION("int max")
		{
			std::vector<uint8_t> _out(4), _verify = {255, 255, 255, 127};
			endianess::toLittle(std::numeric_limits<int32_t>::max(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
		SECTION("int min")
		{
			std::vector<uint8_t> _out(4), _verify = {0, 0, 0, 128};
			endianess::toLittle(std::numeric_limits<int32_t>::min(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
		SECTION("long max")
		{
			std::vector<uint8_t> _out(8), _verify = {255, 255, 255, 255, 255, 255, 255, 127};
			endianess::toLittle(std::numeric_limits<int64_t>::max(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
		SECTION("long min")
		{
			std::vector<uint8_t> _out(8), _verify = {0, 0, 0, 0, 0, 0, 0, 128};
			endianess::toLittle(std::numeric_limits<int64_t>::min(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
	}
	SECTION("toBig")
	{
		SECTION("short max")
		{
			std::vector<uint8_t> _out(2), _verify = {127, 255};
			endianess::toBig(std::numeric_limits<int16_t>::max(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
		SECTION("short min")
		{
			std::vector<uint8_t> _out(2), _verify = {128, 0};
			endianess::toBig(std::numeric_limits<int16_t>::min(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
		SECTION("int max")
		{
			std::vector<uint8_t> _out(4), _verify = {127, 255, 255, 255};
			endianess::toBig(std::numeric_limits<int32_t>::max(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
		SECTION("int min")
		{
			std::vector<uint8_t> _out(4), _verify = {128, 0, 0, 0};
			endianess::toBig(std::numeric_limits<int32_t>::min(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
		SECTION("long max")
		{
			std::vector<uint8_t> _out(8), _verify = {127, 255, 255, 255, 255, 255, 255, 255};
			endianess::toBig(std::numeric_limits<int64_t>::max(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
		SECTION("long min")
		{
			std::vector<uint8_t> _out(8), _verify = {128, 0, 0, 0, 0, 0, 0, 0};
			endianess::toBig(std::numeric_limits<int64_t>::min(), _out.data());
			REQUIRE_THAT(_out, Equals(_verify));
		}
	}
	SECTION("fromLittle")
	{
		SECTION("short max")
		{
			std::vector<uint8_t> _test = {255, 127};
			auto _out = endianess::fromLittle<int16_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int16_t>::max());
		}
		SECTION("short min")
		{
			std::vector<uint8_t> _test = {0, 128};
			auto _out = endianess::fromLittle<int16_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int16_t>::min());
		}
		SECTION("int max")
		{
			std::vector<uint8_t> _test = {255, 255, 255, 127};
			auto _out = endianess::fromLittle<int32_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int32_t>::max());
		}
		SECTION("int min")
		{
			std::vector<uint8_t> _test = {0, 0, 0, 128};
			auto _out = endianess::fromLittle<int32_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int32_t>::min());
		}
		SECTION("long max")
		{
			std::vector<uint8_t> _test = {255, 255, 255, 255, 255, 255, 255, 127};
			auto _out = endianess::fromLittle<int64_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int64_t>::max());
		}
		SECTION("long min")
		{
			std::vector<uint8_t> _test = {0, 0, 0, 0, 0, 0, 0, 128};
			auto _out = endianess::fromLittle<int64_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int64_t>::min());
		}
	}
	SECTION("fromBig")
	{
		SECTION("short max")
		{
			std::vector<uint8_t> _test = {127, 255};
			auto _out = endianess::fromBig<int16_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int16_t>::max());
		}
		SECTION("short min")
		{
			std::vector<uint8_t> _test = {128, 0};
			auto _out = endianess::fromBig<int16_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int16_t>::min());
		}
		SECTION("int max")
		{
			std::vector<uint8_t> _test = {127, 255, 255, 255};
			auto _out = endianess::fromBig<int32_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int32_t>::max());
		}
		SECTION("int min")
		{
			std::vector<uint8_t> _test = {128, 0, 0, 0};
			auto _out = endianess::fromBig<int32_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int32_t>::min());
		}
		SECTION("long max")
		{
			std::vector<uint8_t> _test = {127, 255, 255, 255, 255, 255, 255, 255};
			auto _out = endianess::fromBig<int64_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int64_t>::max());
		}
		SECTION("long min")
		{
			std::vector<uint8_t> _test = {128, 0, 0, 0, 0, 0, 0, 0};
			auto _out = endianess::fromBig<int64_t>(_test.data());
			REQUIRE(_out == std::numeric_limits<int64_t>::min());
		}
	}
	SECTION("float")
	{
		float _test = 0.75f;
		SECTION("big")
		{
			std::vector<uint8_t> _buf(4);
			endianess::toBig<float>(_test, _buf.data());
			auto _out = endianess::fromBig<float>(_buf.data());
			REQUIRE(_out == _test);
		}
		SECTION("little")
		{
			std::vector<uint8_t> _buf(4);
			endianess::toLittle<float>(_test, _buf.data());
			auto _out = endianess::fromLittle<float>(_buf.data());
			REQUIRE(_out == _test);
		}
	}
	SECTION("double")
	{
		double _test = 0.25;
		SECTION("big")
		{
			std::vector<uint8_t> _buf(8);
			endianess::toBig<double>(_test, _buf.data());
			auto _out = endianess::fromBig<double>(_buf.data());
			REQUIRE(_out == _test);
		}
		SECTION("little")
		{
			std::vector<uint8_t> _buf(8);
			endianess::toLittle<double>(_test, _buf.data());
			auto _out = endianess::fromLittle<double>(_buf.data());
			REQUIRE(_out == _test);
		}
	}
}
