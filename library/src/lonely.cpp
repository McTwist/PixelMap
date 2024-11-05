#include "lonely.hpp"

#include "format/region.hpp"

/*
 * Set lonely chunk version to use
 * 0 - Disable it completely, no extra work is done
 * 1 - Goes through all chunks on the map and checks for singles
 * 2 - Goes through all regions if it has possible singles
 */
#define LONELY_CHUNKS 2

#if LONELY_CHUNKS == 1
#include <queue>
#endif

void Lonely::locate(const std::shared_ptr<const region::RegionFile> & region)
{
#if LONELY_CHUNKS == 2
	bool found = false;
	for (int i = 0; i < 1024; ++i)
	{
		int x = (31 & i);
		int z = (i >> 5);
		if (region->containsChunk(x, z))
		{
			// Only right and down, as we have gone through all the rest
			if ((x <= 31 && region->containsChunk(x+1, z)) || ((z <= 31) && region->containsChunk(x, z+1)))
			{
				found = true;
				break;
			}
		}
	}
	if (!found)
		regions.insert({region->x(), region->z()});
#elif LONELY_CHUNKS == 1
	regions.insert({region->x(), region->z()});

	int xx = region->x() * 32;
	int zz = region->z() * 32;
	for (int i = 0; i < 1024; ++i)
	{
		int x = (31 & i);
		int z = (i >> 5);
		if (region->containsChunk(x, z))
		{
			temp_chunks.insert({x + xx, z + zz});
		}
	}
#else
	(void)region;
#endif
}

void Lonely::locate(const utility::PlanePosition & pos)
{
#if LONELY_CHUNKS >= 1
	bool add = false;
	// Note: Iterate all to avoid "edge" cases
	auto posf = temp_chunks.find({pos.x+1, pos.y});
	if (posf != temp_chunks.end())
	{
		chunks.emplace(*posf);
		add = true;
	}
	posf = temp_chunks.find({pos.x, pos.y+1});
	if (posf != temp_chunks.end())
	{
		chunks.emplace(*posf);
		add = true;
	}
	posf = temp_chunks.find({pos.x-1, pos.y});
	if (posf != temp_chunks.end())
	{
		chunks.emplace(*posf);
		add = true;
	}
	posf = temp_chunks.find({pos.x, pos.y-1});
	if (posf != temp_chunks.end())
	{
		chunks.emplace(*posf);
		add = true;
	}
	temp_chunks.emplace(pos);
	if (add)
		chunks.emplace(pos);
#else
	(void)pos;
#endif
}

void Lonely::process()
{
#if LONELY_CHUNKS == 1
	while (!temp_chunks.empty())
	{
		auto it = temp_chunks.begin();
		auto chunk = *it;
		auto lone = chunk;
		// Calculate clusters
		std::queue<decltype(chunk)> que;
		que.push(chunk);
		temp_chunks.erase(chunk);
		int count = 0;
		while (!que.empty())
		{
			++count;
			chunk = que.front();
			que.pop();
			// Find neighbors
			it = temp_chunks.find({chunk.x + 1, chunk.y});
			if (it != temp_chunks.end())
			{
				que.push(*it);
				temp_chunks.erase(it);
			}
			it = temp_chunks.find({chunk.x - 1, chunk.y});
			if (it != temp_chunks.end())
			{
				que.push(*it);
				temp_chunks.erase(it);
			}
			it = temp_chunks.find({chunk.x, chunk.y + 1});
			if (it != temp_chunks.end())
			{
				que.push(*it);
				temp_chunks.erase(it);
			}
			it = temp_chunks.find({chunk.x, chunk.y - 1});
			if (it != temp_chunks.end())
			{
				que.push(*it);
				temp_chunks.erase(it);
			}
			if (que.size() + std::size_t(count) > 1)
				regions.erase({chunk.x >> 5, chunk.y >> 5});
		}
		// Lonely areas
		if (count == 1)
			chunks.insert(lone);
	}
#endif
}

bool Lonely::isLonely(const std::shared_ptr<const region::RegionFile> & region) const
{
#if LONELY_CHUNKS >= 1
	return regions.find({region->x(), region->z()}) != regions.end();
#else
	(void)region;
	return false;
#endif
}

bool Lonely::isLonely(const std::shared_ptr<const region::ChunkData> & chunk) const
{
#if LONELY_CHUNKS == 1
	return chunks.find({chunk->xPos, chunk->zPos}) != chunks.end();
#else
	(void)chunk;
	return false;
#endif
}

bool Lonely::isLonely(const utility::PlanePosition & pos) const
{
	return chunks.find(pos) == chunks.end();
}
