#include "bedrock/factory.hpp"

// Minecraft Bedrock version
enum StorageVersion
{
	STORAGE_VERSION_40 = 40,
};
enum ChunkVersion
{
	CHUNK_VERSION_9 = 9,
};

std::shared_ptr<bedrock::V> bedrock::Factory::create(World & world)
{
    return std::make_shared<bedrock::V>(world);
}
