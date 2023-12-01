#include "render/renderpassbuilder.hpp"

void RenderPassBuilder::add(const std::string &name, RenderPassFunction pass)
{
	if (pass)
		passes.emplace(name, pass);
}

RenderPassFunction RenderPassBuilder::generate(const std::vector<std::string> &names)
{
	std::vector<RenderPassFunction> pass;

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

RenderPassFunction RenderPassBuilder::generate(const std::vector<RenderPassFunction> &pass)
{
	return [passes{std::move(pass)}] (RenderPassData & data)
	{
		for (auto f : passes)
		{
			if (data.pos.y < 0)
				return;
			f(data);
		}
	};
}

