#include "lonely.hpp"

#include "format/region.hpp"

/*
 * Set lonely chunk version to use
 * 0 - Disable it completely, no extra work is done
 * 1 - Goes through all chunks on the map and checks for singles
 * 2 - Goes through all regions if it has possible singles
 */
#define LONELY_CHUNKS 2

#if LONELY_CHUNKS == 2
#include <array>
#elif LONELY_CHUNKS == 1
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
	else
	{
		auto x = region->x();
		auto z = region->z();
		std::array<utility::PlanePosition, 4> poss{{{x-1, z}, {x+1, z}, {x, z-1}, {x, z+1}}};
		utility::PlanePosition p{x, z};
		bool added = false;
		for (auto pos : poss)
		{
			auto it = region2group.find(pos);
			if (it != region2group.end())
			{
				auto next = it->second;
				// Special case: merging two groups
				if (added)
				{
					auto curr = region2group[p];
					// Avoid adding the group to itself
					if (next == curr)
						continue;
					for (auto at : *curr)
					{
						region2group[at] = next;
						next->emplace_back(at);
					}
				}
				else
				{
					region2group[p] = next;
					next->emplace_back(p);
				}
				added = true;
			}
		}
		if (!added)
		{
			auto c = std::make_shared<std::vector<utility::PlanePosition>>();
			c->emplace_back(p);
			region2group[p] = std::move(c);
		}
	}
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
			known_chunks.insert({x + xx, z + zz});
		}
	}
#else
	(void)region;
#endif
}

void Lonely::locate(const utility::PlanePosition & pos)
{
#if LONELY_CHUNKS >= 1
	bool add = true;
	// Note: Iterate all to avoid "edge" cases
	auto posf = known_chunks.find({pos.x+1, pos.y});
	if (posf != known_chunks.end())
	{
		chunks.erase(*posf);
		add = false;
	}
	posf = known_chunks.find({pos.x, pos.y+1});
	if (posf != known_chunks.end())
	{
		chunks.erase(*posf);
		add = false;
	}
	posf = known_chunks.find({pos.x-1, pos.y});
	if (posf != known_chunks.end())
	{
		chunks.erase(*posf);
		add = false;
	}
	posf = known_chunks.find({pos.x, pos.y-1});
	if (posf != known_chunks.end())
	{
		chunks.erase(*posf);
		add = false;
	}
	known_chunks.emplace(pos);
	if (add)
		chunks.emplace(pos);
#else
	(void)pos;
#endif
}

void Lonely::process()
{
#if LONELY_CHUNKS == 2
	std::unordered_set<std::shared_ptr<std::vector<utility::PlanePosition>>> groups;
	for (auto it : region2group)
		groups.emplace(it.second);
	std::size_t max_groups = 0;
	for (auto it : groups)
		if (it->size() > max_groups)
			max_groups = it->size();
	for (auto it : groups)
		if (it->size() < max_groups)
			for (auto pos : *it)
				chunks.emplace(pos);
#elif LONELY_CHUNKS == 1
	while (!known_chunks.empty())
	{
		auto it = known_chunks.begin();
		auto chunk = *it;
		auto lone = chunk;
		// Calculate clusters
		std::queue<decltype(chunk)> que;
		que.push(chunk);
		known_chunks.erase(chunk);
		int count = 0;
		while (!que.empty())
		{
			++count;
			chunk = que.front();
			que.pop();
			// Find neighbors
			it = known_chunks.find({chunk.x + 1, chunk.y});
			if (it != known_chunks.end())
			{
				que.push(*it);
				known_chunks.erase(it);
			}
			it = known_chunks.find({chunk.x - 1, chunk.y});
			if (it != known_chunks.end())
			{
				que.push(*it);
				known_chunks.erase(it);
			}
			it = known_chunks.find({chunk.x, chunk.y + 1});
			if (it != known_chunks.end())
			{
				que.push(*it);
				known_chunks.erase(it);
			}
			it = known_chunks.find({chunk.x, chunk.y - 1});
			if (it != known_chunks.end())
			{
				que.push(*it);
				known_chunks.erase(it);
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
	return chunks.find(pos) != chunks.end();
}
