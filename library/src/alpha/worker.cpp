#include "alpha/worker.hpp"

#include "render/renderpass.hpp"
#include "alpha/v.hpp"
#include "format/alpha.hpp"
#include "util/compression.hpp"
#include "performance.hpp"

#include <spdlog/spdlog.h>

#include <filesystem>
#include <fstream>
#include <regex>

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

alpha::Worker::Worker(std::atomic_bool & _run, const Options & options) :
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

void alpha::Worker::work(const std::string & path, const std::string & output, int32_t)
{
	run = true;

	/*
	 * Note: As the options for a render should be quite hidden, this is
	 * the best option for now. We may have to change it later, but at least
	 * the output is a requirement.
	 */
	settings->path = output;

	alpha::Alpha alpha(path);

	auto drawImage = std::make_shared<WorldRender>(settings);

	drawImage->eventRenderRegion([this](int v)
	{
		func_finishedRender.call(v);
	});

	if (use_lonely)
	{
		// Go through once to get amount
		PERFORMANCE(
		{
			for (const auto & file : alpha)
			{
				func_totalChunks.call(total_chunks += 1);
				func_totalRender.call(total_regions += 1);

				lonely.locate(utility::PlanePosition{file->x(), file->z()});
			}

			lonely.process();
		}, perf.getPerfValue(PERF_Lonely));
	}
	else
	{
		alpha.begin();
	}

	struct FutureChunk
	{
		std::future<std::shared_ptr<ChunkRenderData>> future;
		std::shared_ptr<alpha::AlphaFile> file;
	};

	std::vector<FutureChunk> future_chunk;
	// Note: Approximation
	future_chunk.reserve(alpha.count());

	threadpool::Transaction transaction;

	// Go through each chunk
	for (const auto & file : alpha)
	{
		if (!run)
			break;

		if (lonely.isLonely(utility::PlanePosition{file->x(), file->z()}))
		{
			func_finishedChunk.call(1);
			func_finishedRender.call(1);
			perf.errors.report(ErrorStats::ERROR_LONELY_CHUNKS);
			continue;
		}

		future_chunk.emplace_back(std::move(FutureChunk{transaction.enqueue(1, std::bind(&Worker::workChunk, this, file)), file}));

		/*
		 * As a region is 4096 times larger than a regular chunk, that could be
		 * the value used to make sure that full utilization is being done.
		 */
		if (transaction.size() >= pool.size() * 4096)
			pool.commit(transaction);
	}
	
	pool.commit(transaction);

	pool.wait();

	std::unordered_map<utility::PlanePosition, std::shared_ptr<RegionRender>> regionRenders;
	std::vector<std::future<std::shared_ptr<RegionRenderData>>> future_region;

	if (run)
	{
		for (auto & future : future_chunk)
		{
			auto next = future.future.get();
			utility::PlanePosition pos{future.file->x() >> 5, future.file->z() >> 5};
			auto it = regionRenders.find(pos);
			if (it == regionRenders.end())
				it = regionRenders.insert({pos, std::make_shared<RegionRender>(settings)}).first;
			it->second->add(next);
		}
		func_totalRender.call(total_regions += regionRenders.size());
		for (auto & [pos, drawRegion] : regionRenders)
		{
			future_region.emplace_back(pool.enqueue(0, [drawRegion, pos, this]()
			{
				std::shared_ptr<RegionRenderData> draw;
				PERFORMANCE(
				{
					draw = drawRegion->draw(pos.x, pos.y);
				}, perf.getPerfValue(PERF_RenderRegion));
				perf.regionCounterDecrease();
				return draw;
			}));
		}
	}

	pool.wait();

	for (auto & future : future_region)
	{
		auto regionData = future.get();
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

std::shared_ptr<ChunkRenderData> alpha::Worker::workChunk(std::shared_ptr<alpha::AlphaFile> & file)
{
	std::shared_ptr<ChunkRenderData> draw;
	if (!run)
	{
		return draw;
	}
	bool error = false;
	if (!file->open())
	{
		perf.addErrorString("Unable to open file");
		return draw;
	}
	std::vector<uint8_t> compressed = file->readAll();
	file->close();

	// Uncompress the data
	std::vector<uint8_t> uncompressed;
	{
		PERFORMANCE(
		{
			uncompressed = Compression::loadGZip(compressed);
			if (uncompressed.empty())
			{
				perf.errors.report(ErrorStats::Type::ERROR_COMPRESSION);
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
			alpha::V chunkReader(data);
			// Get all data to be read
			if (reader.parse(uncompressed, chunkReader, NBT::Endianess::BIG) > 0)
			{
				// TODO: Add to statistics
			}
			else
			{
				perf.addErrorString(reader.getError());
				perf.errors.report(ErrorStats::Type::ERROR_PARSE);
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
		perf.errors.report(ErrorStats::Type::ERROR_EMPTY_CHUNKS);
	}

	func_finishedChunk.call(1);

	return draw;
}
