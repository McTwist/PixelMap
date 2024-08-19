#pragma once
#ifndef RENDERPATH_HPP
#define RENDERPATH_HPP

#include "render/renderpassbuilder.hpp"

namespace RenderPass
{

class Default
{
public:
	RenderPassFunction build();
};

class Opaque
{
public:
	RenderPassFunction build();
};

class Heightmap
{
public:
	RenderPassFunction build();
};

class Gray
{
public:
	RenderPassFunction build();
};

class Color
{
public:
	RenderPassFunction build();
};

class Heightline
{
public:
	Heightline(int frequency);
	RenderPassFunction build();
private:
	int frequency;
};

class Night
{
public:
	RenderPassFunction build();
};

class Slice
{
public:
	Slice(int y);
	RenderPassFunction build();
private:
	int y;
};

class Cave
{
public:
	RenderPassFunction build();
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
		DIFFERENCE,
		EXCLUSION,
		HUE,
		SATURATION,
		COLOR,
		LUMINOSITY
	};
	Blend(Mode mode = Mode::LEGACY);
	RenderPassFunction build();
private:
	std::function<utility::RGBA(utility::RGBA, utility::RGBA)> blend;
};

}

#endif // RENDERPATH_HPP
