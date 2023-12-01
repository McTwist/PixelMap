#include "anvil/factory.hpp"

#include "anvil/v3.hpp"
#include "anvil/v13.hpp"
#include "anvil/v16.hpp"
#include "anvil/v18.hpp"
#include "anvil/version.hpp"

std::shared_ptr<anvil::V> anvil::Factory::create(Chunk & chunk)
{
    if (chunk.getDataVersion() < DATA_VERSION_1_13)
        return std::make_shared<anvil::V3>(chunk);
    if (chunk.getDataVersion() < DATA_VERSION_1_16)
        return std::make_shared<anvil::V13>(chunk);
    if (chunk.getDataVersion() < DATA_VERSION_1_18)
        return std::make_shared<anvil::V16>(chunk);
    return std::make_shared<anvil::V18>(chunk);
}
