#pragma once
#ifndef BEDROCK_FACTORY_HPP
#define BEDROCK_FACTORY_HPP

#include "bedrock/v.hpp"

#include <memory>

namespace bedrock
{

class Factory final
{
	Factory() = delete;
public:
	static std::shared_ptr<V> create(World & world);
};

}

#endif // BEDROCK_FACTORY_HPP
