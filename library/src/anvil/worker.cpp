#include "anvil/worker.hpp"

#include "render/renderpass.hpp"
#include "format/anvil.hpp"
#include "anvil/factory.hpp"
#include "util/compression.hpp"
#include "performance.hpp"

#include <spdlog/spdlog.h>

enum PerfE
{
	PERF_Lonely,
	PERF_Decompress,
	PERF_PreParse,
	PERF_Parse,
	PERF_Render,
	PERF_RenderRegion,
	PERF_RenderImage,
};

anvil::Worker::Worker(std::atomic_bool & _run, const Options & options) :
	WorkerBase(_run, options)
{
#ifdef PERF_DEBUG
	perf.createPerfValue("Lonely");
	perf.createPerfValue("Decompress");
	perf.createPerfValue("PreParse");
	perf.createPerfValue("Parse");
	perf.createPerfValue("Render");
	perf.createPerfValue("Render Region");
	perf.createPerfValue("Render Image");
#endif
}

void anvil::Worker::work(const std::string & path, const std::string & output, int32_t dimension)
{
	run = true;

	/*
	 * Note: As the options for a render should be quite hidden, this is
	 * the best option for now. We may have to change it later, but at least
	 * the output is a requirement.
	 */
	settings->path = output;

	anvil::Anvil anvil(path);

	auto drawImage = std::make_shared<WorldRender>(settings);

	drawImage->eventRenderRegion([this](int v)
	{
		func_finishedRender.call(v);
	});

	// Go through once to get amount
	PERFORMANCE(
	{
		for (auto region : anvil)
		{
			auto amount = region->getAmountChunks();
			if (amount == 0)
				continue;
			func_totalChunks.call(total_chunks += amount);
			func_totalRender.call(total_regions += amount);

			lonely.locate(region);
		}

		lonely.process();
	}, perf.getPerfValue(PERF_Lonely));

	std::vector<std::future<std::future<std::shared_ptr<RegionRenderData>>>> futures;

	threadpool::Transaction transaction;
	int i = 0;

	// Go through each region
	for (auto region : anvil)
	{
		if (!run)
			break;
		// Avoid handling regions that is empty
		if (region->getAmountChunks() == 0)
			continue;

		if (lonely.isLonely(region))
		{
			func_finishedChunk.call(region->getAmountChunks());
			func_finishedRender.call(region->getAmountChunks());
			continue;
		}

		region->close();

		perf.regionCounterIncrease();

		/*
		 * Note: Prioritize second level to reduce the amount of memory loaded
		 * at the same time. This also ensures that each region file is parsed
		 * before going to the next one.
		 */
		futures.emplace_back(transaction.enqueue(i, std::bind(&Worker::workRegion, this, region, i)));
		i += 2;

		if (transaction.size() >= pool.size())
			pool.commit(transaction);
	}
	
	pool.commit(transaction);

	pool.wait();

	for (auto & future : futures)
	{
		auto next = future.get();
		auto regionData = next.get();
		drawImage->add(regionData);
	}

	func_finishedChunks.call();

	if (run)
	{
		PERFORMANCE(
		{
			drawImage->draw();
		}, perf.getPerfValue(PERF_RenderImage));

		func_finishedRenders.call();
	}

	run = false;

	perf.print();
}

std::future<std::shared_ptr<RegionRenderData>> anvil::Worker::workRegion(std::shared_ptr<anvil::AnvilRegion> region, int i)
{
	auto x = region->x();
	auto z = region->z();

	utility::PlanePosition pos(x, z);

	std::shared_ptr<RegionRenderData> draw;
	RegionRender drawRegion(settings);
	std::vector<std::shared_future<std::shared_ptr<ChunkRenderData>>> futures;
	futures.reserve(region->getAmountChunks());

	if (settings->mode == Render::DRAW_REGION_TINY)
	{
		PERFORMANCE(
		{
			draw = drawRegion.draw(pos.x, pos.y);
		}, perf.getPerfValue(PERF_RenderRegion));
		perf.regionCounterDecrease();

		func_finishedChunk.call(region->getAmountChunks());

		region->close();
		std::promise<std::shared_ptr<RegionRenderData>> promise;
		promise.set_value(draw);
		return promise.get_future();
	}

	threadpool::Transaction transaction;

	// Go through each chunk for each region
	for (auto chunk : *region)
	{
		if (!run)
			break;
		if (!chunk)
		{
			perf.addErrorString("Chunk not loaded");
			perf.addErrorString(region->getLastError());
			continue;
		}

		// Found lonely chunks
		// Note: Read only, to avoid corruption
		if (lonely.isLonely(chunk))
		{
			func_finishedChunk.call(1);
			func_finishedRender.call(1);
			continue;
		}

		if (settings->mode == Render::DRAW_CHUNK_TINY)
		{
			ChunkRender drawChunk(settings);

			PERFORMANCE(
			{
				Chunk dummy;
				auto d = drawChunk.draw(dummy, renderPass);
				std::promise<std::shared_ptr<ChunkRenderData>> promise;
				promise.set_value(d);
				futures.emplace_back(promise.get_future());
			}, perf.getPerfValue(PERF_Render));

			func_finishedChunk.call(1);

			continue;
		}

		/*
		 * If there is more regions than chunks, then it is more worth
		 * to process a whole region instead of several smaller chunks,
		 * which adds more overhead and may choke the pool.
		 * However, on the case of less regions than threads, the
		 * benefits are neglectable on smaller worlds.
		 */
		/*
		 * Note: This seems to be most effective when each region got its own image file.
		 * This may be due to the blocking mechanism when saving the rendered image.
		 */
		if (total_regions < pool.size())
			futures.emplace_back(transaction.enqueue(i+1, std::bind(&Worker::workChunk, this, chunk)));
		else
		{
			std::promise<std::shared_ptr<ChunkRenderData>> promise;
			promise.set_value(workChunk(chunk));
			futures.emplace_back(promise.get_future());
		}

		if (transaction.size() >= pool.size())
			pool.commit(transaction);
	}

	pool.commit(transaction);
	
	region->close();

	std::future<std::shared_ptr<RegionRenderData>> future;
	if (run)
	{
		future = pool.enqueue(i, [pos, region, futures, this]()
		{
			RegionRender drawRegion(settings);
			for (auto & future : futures)
			{
				future.wait();
				drawRegion.add(future.get());
			}
			region->clear();
			std::shared_ptr<RegionRenderData> draw;
			if (!run)
				return draw;

			PERFORMANCE(
			{
				draw = drawRegion.draw(pos.x, pos.y);
			}, perf.getPerfValue(PERF_RenderRegion));
			perf.regionCounterDecrease();
			return draw;
		});
	}

	return future;
}

std::shared_ptr<ChunkRenderData> anvil::Worker::workChunk(std::shared_ptr<anvil::ChunkData> chunk)
{
	std::shared_ptr<ChunkRenderData> draw;
	if (!run)
	{
		return draw;
	}
	bool error = false;
	// Uncompress the data
	std::vector<uint8_t> uncompressed;
	{
		PERFORMANCE(
		{
			switch (chunk->compression_type)
			{
			case ChunkData::COMPRESSION_ZLIB:
				uncompressed = Compression::loadZLib(chunk->data);
				if (uncompressed.empty())
				{
					perf.errors.report(ErrorStats::ERROR_COMPRESSION);
					error = true;
				}
				break;
			case ChunkData::COMPRESSION_GZIP:
				uncompressed = Compression::loadGZip(chunk->data);
				if (uncompressed.empty())
				{
					perf.errors.report(ErrorStats::ERROR_COMPRESSION);
					error = true;
				}
				break;
			default:
				perf.errors.report(ErrorStats::ERROR_TYPE);
				error = true;
			}
		}, perf.getPerfValue(PERF_Decompress));
	}

	if (error)
	{
		perf.addErrorString("Decompression error");
		return draw;
	}

	NBT::Reader reader;
	Chunk data;

	{
		PERFORMANCE(
		{
			anvil::V chunkVersion(data);
			if (reader.parse(uncompressed, chunkVersion, NBT::ENDIAN_BIG) <= 0)
			{
				perf.addErrorString(reader.getError());
				perf.errors.report(ErrorStats::ERROR_PARSE);
				error = true;
			}
		}, perf.getPerfValue(PERF_PreParse));
		if (error)
		{
			perf.addErrorString("Pre-parse error");
			return draw;
		}
		PERFORMANCE(
		{
			auto chunkReader = anvil::Factory::create(data);
			// Get all data to be read
			if (reader.parse(uncompressed, *chunkReader, NBT::ENDIAN_BIG) > 0)
			{
				// TODO: Add to statistics
			}
			else
			{
				perf.addErrorString(reader.getError());
				perf.errors.report(ErrorStats::ERROR_PARSE);
				error = true;
			}
		}, perf.getPerfValue(PERF_Parse));
	}

	if (error)
	{
		perf.addErrorString("Parse error");
		return draw;
	}

	if (data.isValid())
	{
		PERFORMANCE(
		{
			ChunkRender drawChunk(settings);
			draw = drawChunk.draw(data, renderPass);
		}, perf.getPerfValue(PERF_Render));
	}
	else
	{
		perf.errors.report(ErrorStats::ERROR_EMPTY_CHUNKS);
	}

	func_finishedChunk.call(1);

	return draw;
}
