#pragma once
#ifndef PASSBUILDER_HPP
#define PASSBUILDER_HPP

#include "render/utility.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <vector>
#include <memory>

class Chunk;

struct RenderPassData
{
	const std::vector<utility::RGBA> & palette; // in
	const Chunk & chunk; // in
	const utility::Direction & dir; // in
	utility::Vector pos; // inout
	utility::RGBA color; // inout
};

typedef std::function<void(RenderPassData &)> RenderPassFunction;

class PassBuilder
{
public:
	virtual ~PassBuilder() = default;

	virtual RenderPassFunction build() = 0;
};

#endif // PASSBUILDER_HPP
