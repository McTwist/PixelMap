#pragma once
#ifndef BLOCKPASSBUILDER_HPP
#define BLOCKPASSBUILDER_HPP

#include "render/utility.hpp"
#include "render/passbuilder.hpp"

#include <functional>
#include <unordered_map>
#include <string>

class BlockPassBuilder
{
public:
	void add(const std::string & name, BlockPassFunction pass);

	BlockPassFunction generate(const std::vector<std::string> & names);
	BlockPassFunction generate(const std::vector<BlockPassFunction> & passes);

private:
	std::unordered_map<std::string, BlockPassFunction> passes;
};

#endif // BLOCKPASSBUILDER_HPP
