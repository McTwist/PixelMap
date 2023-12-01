#pragma once
#ifndef RENDERPASSBUILDER_HPP
#define RENDERPASSBUILDER_HPP

#include "render/utility.hpp"
#include "chunk.hpp"

#include <glm/glm.hpp>

#include <functional>
#include <unordered_map>
#include <string>

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

class RenderPassBuilder
{
public:
	void add(const std::string & name, RenderPassFunction pass);

	RenderPassFunction generate(const std::vector<std::string> & names);
	RenderPassFunction generate(const std::vector<RenderPassFunction> & passes);

private:
	std::unordered_map<std::string, RenderPassFunction> passes;
};

#endif // RENDERPASSBUILDER_HPP
