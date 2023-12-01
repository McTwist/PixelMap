#pragma once
#ifndef RENDERPATH_HPP
#define RENDERPATH_HPP

#include "render/renderpassbuilder.hpp"

namespace RenderPass
{

class PassBuilder
{
public:
	virtual ~PassBuilder() = delete;

	virtual RenderPassFunction build() = 0;
};

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
	RenderPassFunction build();
};

}

#endif // RENDERPATH_HPP
