#pragma once
#ifndef BLOCKPASS_HPP
#define BLOCKPASS_HPP

#include "render/blockpassbuilder.hpp"

namespace BlockPass
{

class Default
{
public:
	BlockPassFunction build();
};

class Opaque
{
public:
	BlockPassFunction build();
};

class Heightmap
{
public:
	BlockPassFunction build();
};

class Gray
{
public:
	BlockPassFunction build();
};

class Color
{
public:
	BlockPassFunction build();
};

class Heightline
{
public:
	Heightline(int frequency);
	BlockPassFunction build();
private:
	int frequency;
};

class Night
{
public:
	BlockPassFunction build();
};

class Slice
{
public:
	Slice(int y);
	BlockPassFunction build();
private:
	int y;
};

class Cave
{
public:
	BlockPassFunction build();
};

class Blend
{
public:
	enum class Mode {
		LEGACY,
		NORMAL,
		MULTIPLY,
		SCREEN,
		OVERLAY,
		DARKEN,
		LIGHTEN,
		COLOR_DODGE,
		COLOR_BURN,
		HARD_LIGHT,
		SOFT_LIGHT,
		DIFFERENCE_,
		EXCLUSION,
		HUE,
		SATURATION,
		COLOR,
		LUMINOSITY
	};
	Blend(Mode mode = Mode::LEGACY);
	BlockPassFunction build();
private:
	std::function<utility::RGBA(utility::RGBA, utility::RGBA)> blend;
};

}

#endif // BLOCKPASS_HPP
