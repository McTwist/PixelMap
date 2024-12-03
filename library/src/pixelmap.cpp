#include "pixelmap.hpp"

#include "delayedaccumulator.hpp"
#include "platform.hpp"
#include "performance.hpp"
#include "alpha/worker.hpp"
#include "beta/worker.hpp"
#include "anvil/worker.hpp"
#include "bedrock/worker.hpp"
#include "minecraft.hpp"

#include <spdlog/spdlog.h>

#include <filesystem>

struct InOutData
{
	std::string path;
	std::string output;
	int32_t dimension;
};

PixelMap::PixelMap()
	: run(false)
{
	// Create main thread
	worker.start([this](Any datum)
	{
		auto data = datum.get<InOutData>();
		work(data.path, data.output, data.dimension);
	});
}

PixelMap::~PixelMap()
{
}

void PixelMap::eventTotalChunks(std::function<void(int)> && func)
{
	func_totalChunks.add(std::move(func));
}

void PixelMap::eventFinishedChunk(std::function<void(int)> && func)
{
	func_finishedChunk.add(std::move(func));
}

void PixelMap::eventTotalRender(std::function<void (int)> &&func)
{
	func_totalRender.add(std::move(func));
}

void PixelMap::eventFinishedRender(std::function<void (int)> &&func)
{
	func_finishedRender.add(std::move(func));
}

void PixelMap::eventDone(std::function<void()> && func)
{
	func_done.add(std::move(func));
}

void PixelMap::set(const Options & _options)
{
	options = _options;
}

void PixelMap::start(const std::string & path, const std::string & output, int32_t dimension)
{
	worker.enqueue(InOutData{path, output, dimension});
}

void PixelMap::stop()
{
	if (worker.idle())
		return;
	run = false;
}

void PixelMap::wait()
{
	worker.wait();
}

bool PixelMap::done()
{
	return worker.idle();
}

// Work the path specified
void PixelMap::work(const std::string & path, const std::string & output, int32_t dimension)
{
	std::string real_path = path;
	std::unique_ptr<WorkerBase> works;
	switch (Minecraft::determineSaveVersion(path))
	{
	case Minecraft::SaveVersion::ANVIL:
		if (std::filesystem::is_directory(platform::path::join(path, "region")))
			real_path = Minecraft::JE::getDimensionPath(path, dimension);
		works = std::make_unique<anvil::Worker>(run, options);
		break;
	case Minecraft::SaveVersion::LEVELDB:
		if (std::filesystem::is_directory(platform::path::join(path, "db")))
			real_path = platform::path::join(path, "db");
		works = std::make_unique<bedrock::Worker>(run, options);
		break;
	case Minecraft::SaveVersion::BETA:
		if (std::filesystem::is_directory(platform::path::join(path, "region")))
			real_path = Minecraft::JE::getDimensionPath(path, dimension);
		works = std::make_unique<beta::Worker>(run, options);
		break;
	case Minecraft::SaveVersion::ALPHA:
		if (dimension)
			real_path = platform::path::join(path, fmt::format("DIM{:d}", dimension));
		works = std::make_unique<alpha::Worker>(run, options);
		break;
	case Minecraft::SaveVersion::UNKNOWN:
		return;
	}

	if (!works->valid())
		return;

	constexpr auto delay = 1.f / 20.f;

	DelayedAccumulator delay_chunks([this](int v)
	{
		func_finishedChunk.call(v);
	}, delay);

	DelayedAccumulator delay_render([this](int v)
	{
		func_finishedRender.call(v);
	}, delay);

	works->eventTotalChunks([this](int a) { func_totalChunks.call(a); });
	works->eventTotalRender([this](int a) { func_totalRender.call(a); });
	works->eventFinishedChunk([&delay_chunks](int a) { delay_chunks.add(a); });
	works->eventFinishedRender([&delay_render](int a) { delay_render.add(a); });
	works->eventFinishedChunk([&delay_chunks]() { delay_chunks.flush(); });
	works->eventFinishedRender([&delay_render]() { delay_render.flush(); });

#ifdef PERF_DEBUG
	Timer<double> total;
	total.start();
#endif

	works->work(real_path, output, dimension);

	func_done.call();

#ifdef PERF_DEBUG
	auto t = total.elapsed<>();
	spdlog::info("Total: {}", t);
#endif
}
