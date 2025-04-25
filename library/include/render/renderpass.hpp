#pragma once
#ifndef RENDERPASS_HPP
#define RENDERPASS_HPP

#include "render/renderpassdeclare.hpp"

struct RenderSettings;

class ChunkPassFactory
{
public:
	static ChunkPassFunction create(std::shared_ptr<RenderSettings> setting, BlockPassFunction func);
};

class RegionPassFactory
{
public:
	static RegionPassFunction create(std::shared_ptr<RenderSettings> setting);
};

class WorldPassFactory
{
public:
	static WorldPassFunction create(std::shared_ptr<RenderSettings> setting);
};

#endif // RENDERPASS_HPP
