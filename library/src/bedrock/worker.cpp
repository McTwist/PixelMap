#include "bedrock/worker.hpp"

#include "format/leveldb.hpp"
#include "bedrock/world.hpp"
#include "bedrock/factory.hpp"
#include "performance.hpp"

#include <spdlog/spdlog.h>

/**
 * @brief Priority when put in queue
 */
enum QueuePriority
{
	QP_MAP = 0,
	QP_FILE = 1,
	QP_CHUNK = 2
};

enum PerfE
{
	PERF_Lonely,
	PERF_Parse,
	PERF_Render,
	PERF_Merge,
	PERF_RenderRegion,
	PERF_RenderImage,
};

bedrock::Worker::Worker(std::atomic_bool & _run, const Options & options) :
	WorkerBase(_run, options)
{
	// Load block color
	if (options.has("lightsource"))
		light_source.read(options.get<std::string>("lightsource"));
	else
		light_source.read();

	if (!light_source.hasLightSources())
	{
		spdlog::warn("Using default lights");
		if (!light_source.read())
			spdlog::error("Internal error: No lights found, contact developer");
	}

	night_mode = options.get<bool>("night", false);

#ifdef PERF_DEBUG
	perf.createPerfValue("Lonely");
	perf.createPerfValue("Parse");
	perf.createPerfValue("Render");
	perf.createPerfValue("Merge");
	perf.createPerfValue("Render Region");
	perf.createPerfValue("Render Image");
#endif
}

void bedrock::Worker::work(const std::string & path, const std::string & output, int32_t dimension)
{
	run = true;

	/*
	 * Note: As the options for a render should be quite hidden, this is
	 * the best option for now. We may have to change it later, but at least
	 * the output is a requirement.
	 */
	settings->path = output;

	LevelDB::LevelDB leveldb(path);

	auto drawImage = std::make_shared<WorldRender>(settings);

	drawImage->eventRenderRegion([this](int v)
	{
		func_finishedRender.call(v);
	});

	func_totalChunks.call(total_chunks = leveldb.size());

	std::queue<std::shared_future<std::shared_ptr<World>>> futures;

	// Go through each file
	for (auto file : leveldb)
	{
		if (!run)
			break;

		file->close();

		perf.regionCounterIncrease();

		/*
		 * Note: Prioritize second level to reduce the amount of memory loaded
		 * at the same time. This also ensures that each file is parsed before
		 * going to the next one.
		 */

		futures.emplace(pool.enqueue(1, std::bind(&Worker::workFile, this, file, dimension)));
	}

	for (int prio = 0; futures.size() > 1; --prio)
	{
		auto f1 = futures.front();
		futures.pop();
		auto f2 = futures.front();
		futures.pop();
		futures.emplace(pool.enqueue(prio, std::bind(&Worker::mergeWorlds, this, f1, f2)));
	}

	// Manually handle log file
	auto file = leveldb.getLog();
	auto world = std::make_shared<World>(file->file(), dimension);
	{
		bool error = 0;
		LevelDB::LogReader reader;
		auto block = file->readAll();
		auto worldReader = bedrock::Factory::create(*world);
		PERFORMANCE(
		{
			if (reader.parse(block, *worldReader) == 0)
			{
				// TODO: Statistics
			}
			else
			{
				perf.addErrorString(reader.getError());
				perf.errors.report(ErrorStats::ERROR_PARSE);
				error = true;
			}
		}, perf.getPerfValue(PERF_Parse));

		if (!error)
		{
			func_finishedChunk.call(1);

			PERFORMANCE(
			{
				ChunkRender renderChunk(settings);
				world->draw([this, &renderChunk](const Chunk & chunk) {
					return renderChunk.draw(chunk, renderPass);
				});
			}, perf.getPerfValue(PERF_Render));
		}
	}
	file->close();

	pool.wait();

	func_finishedChunks.call();

	auto future = futures.front();
	futures.pop();
	world->merge(*future.get());

	if (run)
	{
		std::unordered_map<utility::RegionPosition, std::vector<std::shared_ptr<ChunkRenderData>>> regions;
		/*if (use_lonely)
		{
			PERFORMANCE(
			{
				for (auto & it : world->render())
					lonely.locate(it.first);
				lonely.process();
			}, perf.getPerfValue(PERF_Lonely));
		}*/

		for (auto & it : world->render())
			//if (!lonely.isLonely(it.first))
				regions[utility::coord::toRegion(it.first)].emplace_back(it.second);

		std::vector<std::shared_future<std::shared_ptr<RegionRenderData>>> futures;
		for (auto & it : regions)
			futures.emplace_back(pool.enqueue(1, std::bind(&Worker::renderRegion, this, it.first, it.second)));

		func_totalRender.call(world->size());

		pool.wait();

		regions.clear();

		for (auto f : futures)
		{
			auto d = f.get();
			drawImage->add(d);
		}
		PERFORMANCE(
		{
			drawImage->draw();
		}, perf.getPerfValue(PERF_RenderImage));

		func_finishedRenders.call();
	}

	run = false;

	perf.print();
}

std::shared_ptr<bedrock::World> bedrock::Worker::workFile(std::shared_ptr<LevelDB::LevelFile> file, int32_t dimension)
{
	bool error = 0;
	auto world = std::make_shared<World>(file->file(), dimension);
	LevelDB::LevelReader reader;
	auto block = file->readAll();
	auto worldReader = bedrock::Factory::create(*world);
	PERFORMANCE(
	{
		if (reader.parse(block, *worldReader) == 0)
		{
			// TODO: Statistics
		}
		else
		{
			perf.addErrorString(reader.getError());
			perf.errors.report(ErrorStats::ERROR_PARSE);
			error = true;
		}
	}, perf.getPerfValue(PERF_Parse));

	file->close();

	if (error)
	{
		return world;
	}

	if (night_mode)
		world->generateBlockLight(light_source);

	func_finishedChunk.call(1);

	PERFORMANCE(
	{
		ChunkRender renderChunk(settings);
		world->draw([this, &renderChunk](const Chunk & chunk) {
			return renderChunk.draw(chunk, renderPass);
		});
	}, perf.getPerfValue(PERF_Render));

	return world;
}

std::shared_ptr<bedrock::World> bedrock::Worker::mergeWorlds(std::shared_future<std::shared_ptr<bedrock::World>> f1, std::shared_future<std::shared_ptr<bedrock::World>> f2)
{
	f1.wait();
	f2.wait();
	auto w1 = f1.get();
	auto w2 = f2.get();
	decltype(w1) w;
	PERFORMANCE(
	{
		if (*w1 < *w2)
		{
			w2->merge(*w1);
			w = w2;
		}
		else
		{
			w1->merge(*w2);
			w = w1;
		}
	}, perf.getPerfValue(PERF_Merge));
	return w;
}

std::shared_ptr<RegionRenderData> bedrock::Worker::renderRegion(utility::RegionPosition pos, std::vector<std::shared_ptr<ChunkRenderData>> chunks)
{
	RegionRender renderRegion(settings);
	for (auto chunk : chunks)
		renderRegion.add(chunk);
	func_finishedRender.call(1);
	std::shared_ptr<RegionRenderData> regionData;
	PERFORMANCE(
	{
		regionData = renderRegion.draw(pos.x, pos.y);
	}, perf.getPerfValue(PERF_RenderRegion));
	return regionData;
}

