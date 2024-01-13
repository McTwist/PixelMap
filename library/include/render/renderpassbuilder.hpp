#pragma once
#ifndef RENDERPASSBUILDER_HPP
#define RENDERPASSBUILDER_HPP

#include "render/utility.hpp"
#include "render/passbuilder.hpp"

#include <functional>
#include <unordered_map>
#include <string>

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
