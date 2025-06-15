#include "catch2/catch_test_macros.hpp"

#include "render/color.hpp"
#include "color-print.hpp"


TEST_CASE("color", "[utility]")
{
	using namespace utility;
	using namespace color;
	RGBA background{15, 191, 47, 143};
	RGBA foreground{127, 31, 63, 92};
	SECTION("normal zero")
	{
		CHECK(normal(RGBA(0, 0, 0, 0), RGBA(0, 0, 0, 0)) == RGBA(0, 0, 0, 0));
	}
	SECTION("normal")
	{
		CHECK(normal(background, foreground) == RGBA(71, 111, 55, 183));
	}
	SECTION("multiply")
	{
		CHECK(multiply(background, foreground) == RGBA(38, 109, 40, 183));
	}
	SECTION("screen")
	{
		CHECK(screen(background, foreground) == RGBA(73, 158, 65, 183));
	}
	SECTION("overlay")
	{
		CHECK(overlay(background, foreground) == RGBA(40, 142, 44, 183));
	}
	SECTION("darken")
	{
		CHECK(darken(background, foreground) == RGBA(40, 111, 51, 183));
	}
	SECTION("lighten")
	{
		CHECK(lighten(background, foreground) == RGBA(71, 156, 55, 183));
	}
	SECTION("color_dodge")
	{
		CHECK(color_dodge(background, foreground) == RGBA(44, 163, 55, 183));
	}
	SECTION("color_burn")
	{
		CHECK(color_burn(background, foreground) == RGBA(36, 102, 37, 183));
	}
	SECTION("hard_light")
	{
		CHECK(hard_light(background, foreground) == RGBA(40, 115, 44, 183));
	}
	SECTION("soft_light")
	{
		CHECK(soft_light(background, foreground) == RGBA(40, 146, 45, 183));
	}
	SECTION("difference")
	{
		CHECK(difference(background, foreground) == RGBA(67, 147, 42, 183));
	}
	SECTION("exclusion")
	{
		CHECK(exclusion(background, foreground) == RGBA(71, 151, 62, 183));
	}
	SECTION("hue")
	{
		CHECK(hue(background, foreground) == RGBA(103, 120, 72, 183));
	}
	SECTION("saturation")
	{
		CHECK(saturation(background, foreground) == RGBA(54, 147, 60, 183));
	}
	SECTION("color")
	{
		CHECK(color::color(background, foreground) == RGBA(88, 127, 72, 183));
	}
	SECTION("luminosity")
	{
		CHECK(luminosity(background, foreground) == RGBA(36, 131, 42, 183));
	}
}
