#pragma once
#ifndef ANVIL_FACTORY_HPP
#define ANVIL_FACTORY_HPP

#include "anvil/v.hpp"

#include <memory>

namespace anvil
{

class Factory final
{
	Factory() = delete;
public:
	static std::shared_ptr<V> create(Chunk & chunk);
};

}

#endif // ANVIL_FACTORY_HPP
