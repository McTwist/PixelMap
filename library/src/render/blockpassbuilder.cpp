#include "render/blockpassbuilder.hpp"

void BlockPassBuilder::add(const std::string &name, BlockPassFunction pass)
{
	if (pass)
		passes.emplace(name, pass);
}

BlockPassFunction BlockPassBuilder::generate(const std::vector<std::string> &names)
{
	std::vector<BlockPassFunction> pass;

	for (auto name : names)
	{
		auto it = passes.find(name);
		if (it != passes.end())
		{
			pass.push_back(it->second);
		}
	}

	return generate(pass);
}

BlockPassFunction BlockPassBuilder::generate(const std::vector<BlockPassFunction> &pass)
{
	return [passes{std::move(pass)}] (BlockPassData & data)
	{
		for (auto f : passes)
		{
			if (data.pos.y < 0)
				return;
			f(data);
		}
	};
}

