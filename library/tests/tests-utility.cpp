#include "catch2/catch.hpp"

#include "render/utility.hpp"
#include "color-print.hpp"


TEST_CASE("utility", "[utility]")
{
	using namespace utility;
	SECTION("color")
	{
		using namespace color;
		auto a = RGBA(255, 255, 255, 0), b = RGBA(0, 0, 0, 255);
		SECTION("blend")
		{
			SECTION("transparency")
			{
				REQUIRE(blend(a, b, 0) == b);
				REQUIRE(blend(b, a, 0) == b);
			}
			SECTION("opaque background")
			{
				auto c = a;
				c.a = 127;
				REQUIRE(blend(c, b, 0).a == 255);
			}
			// Note: It is a bit difficult to add more tests
		}
		SECTION("interpolate")
		{
			SECTION("zero")
			{
				REQUIRE(interpolate(a, b, 0) == a);
			}
			SECTION("one")
			{
				REQUIRE(interpolate(a, b, 1) == RGBA(b.r, b.g, b.b, a.a));
			}
			SECTION("half")
			{
				REQUIRE(interpolate(a, b, 0.5f) == RGBA(127, 127, 127, a.a));
			}
		}
	}
	SECTION("space")
	{
		using namespace space;
		Vector a{10, 9, 8};
		SECTION("next")
		{
			SECTION("down")
			{
				auto ray = RayTracing(a, {0, -1, 0});
				REQUIRE(ray.next() == Vector{10, 8, 8});
			}
			SECTION("up")
			{
				auto ray = RayTracing(a, {0, 1, 0});
				REQUIRE(ray.next() == Vector{10, 10, 8});
			}
			SECTION("left")
			{
				auto ray = RayTracing(a, {1, 0, 0});
				REQUIRE(ray.next() == Vector{11, 9, 8});
			}
			SECTION("right")
			{
				auto ray = RayTracing(a, {-1, 0, 0});
				REQUIRE(ray.next() == Vector{9, 9, 8});
			}
			SECTION("forward")
			{
				auto ray = RayTracing(a, {0, 0, 1});
				REQUIRE(ray.next() == Vector{10, 9, 9});
			}
			SECTION("back")
			{
				auto ray = RayTracing(a, {0, 0, -1});
				REQUIRE(ray.next() == Vector{10, 9, 7});
			}
			SECTION("isometric")
			{
				auto ray = RayTracing(a, {-1, -1, -1});
				auto b = ray.next();
				b = ray.next();
				b = ray.next();
				REQUIRE(b == Vector{9, 8, 7});
			}
		}
		SECTION("to")
		{
			REQUIRE(to(a) == TilePosition{10, 9, 8});
		}
		SECTION("toPlane")
		{
			REQUIRE(toPlane(a) == PlanePosition{10, 8});
		}
		SECTION("proj")
		{
			SECTION("min")
			{
				REQUIRE(proj(-1, -1, 1, 0, 256) == 0);
			}
			SECTION("max")
			{
				REQUIRE(proj(1, -1, 1, 0, 256) == 256);
			}
			SECTION("middle")
			{
				REQUIRE(proj(0, -1, 1, 0, 256) == 128);
			}
		}
	}
	SECTION("coord")
	{
		using namespace coord;
		auto linearDiv = [](int v, int d, int m) {
			return (v - m) / d + m / d;
		};
		SECTION("toRegion")
		{
			SECTION("positive")
			{
				auto x = GENERATE(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 31);
				CAPTURE(x);
				REQUIRE(toRegion(ChunkPosition{x}) == RegionPosition{linearDiv(x, 32, 0)});
			}
			SECTION("negative")
			{
				auto x = GENERATE(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -32);
				CAPTURE(x);
				REQUIRE(toRegion(ChunkPosition{x}) == RegionPosition{linearDiv(x, 32, -32)});
			}
		}
		SECTION("toChunk")
		{
			SECTION("positive")
			{
				auto x = GENERATE(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 31);
				CAPTURE(x);
				REQUIRE(toChunk(SectionPosition{x}) == ChunkPosition{x});
			}
			SECTION("negative")
			{
				auto x = GENERATE(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -32);
				CAPTURE(x);
				REQUIRE(toChunk(SectionPosition{x}) == ChunkPosition{x});
			}
		}
		SECTION("toSection")
		{
			SECTION("positive")
			{
				auto x = GENERATE(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 31);
				CAPTURE(x);
				REQUIRE(toSection(BlockPosition{x}) == SectionPosition{linearDiv(x, 16, 0)});
			}
			SECTION("negative")
			{
				auto x = GENERATE(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16, -32);
				CAPTURE(x);
				REQUIRE(toSection(BlockPosition{x}) == SectionPosition{linearDiv(x, 16, -32)});
			}
		}
		SECTION("modSection")
		{
			SECTION("postive")
			{
				auto x = GENERATE(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
				CAPTURE(x);
				REQUIRE(modSection(BlockPosition{x}) == BlockPosition{x % 16});
			}
			SECTION("negative")
			{
				auto x = GENERATE(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16);
				CAPTURE(x);
				REQUIRE(modSection(BlockPosition{x}) == BlockPosition{(x + 32) % 16});
			}
		}
		SECTION("toSection")
		{
			SECTION("postive")
			{
				auto x = GENERATE(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
				CAPTURE(x);
				REQUIRE(toSection(x) == linearDiv(x, 16, 0));
			}
			SECTION("negative")
			{
				auto x = GENERATE(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16);
				CAPTURE(x);
				REQUIRE(toSection(x) == linearDiv(x, 16, -32));
			}
		}
	}
	SECTION("math")
	{
		using namespace math;
		SECTION("mod")
		{
			auto n = GENERATE(2, 4, 8);
			auto i = GENERATE(0, 1, 2, 3, 4, 5, 6, 7, 8);
			SECTION("default modular")
			{
				CAPTURE(n, i);
				REQUIRE(mod(i, n) == std::size_t(i) % n);
			}
			SECTION("negative values")
			{
				i = -i;
				CAPTURE(n, i);
				REQUIRE(mod(i, n) == std::size_t(i + (12 * n)) % n);
			}
		}
		SECTION("indexMod2d")
		{
			SECTION("default index mod")
			{
				REQUIRE(indexMod2d(4, 0, 0) == 0);
			}
			auto n = GENERATE(2, 4, 8);
			auto i = GENERATE(1, 2, 3, 4, 5, 6, 7, 8);
			SECTION("+x")
			{
				CAPTURE(n, i);
				REQUIRE(indexMod2d(n, i, 0) == std::size_t(i) % n);
			}
			SECTION("+z")
			{
				CAPTURE(n, i);
				REQUIRE(indexMod2d(n, 0, i) == std::size_t(i % n) * n);
			}
			SECTION("-x")
			{
				i = -i;
				CAPTURE(n, i);
				REQUIRE(indexMod2d(n, i, 0) == std::size_t(i + (12 * n)) % n);
			}
			SECTION("-z")
			{
				i = -i;
				CAPTURE(n, i);
				REQUIRE(indexMod2d(n, 0, i) == std::size_t((i + (12 * n)) % n) * n);
			}
		}
		SECTION("index2d")
		{
			SECTION("default index")
			{
				REQUIRE(index2d(4, 0, 0) == 0);
			}
			auto i = GENERATE(1, 2, 3, 4, 5, 6, 7, 8);
			SECTION("x")
			{
				REQUIRE(index2d(4, i, 0) == std::size_t(i));
			}
			SECTION("z")
			{
				REQUIRE(index2d(4, 0, i) == std::size_t(i) * 4);
			}
			SECTION("xz")
			{
				REQUIRE(index2d(4, i, i) == std::size_t(i) * 4 + std::size_t(i));
			}
		}
	}
}
