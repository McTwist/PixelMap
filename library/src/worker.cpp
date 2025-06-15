#include "worker.hpp"

#include "render/blockpass.hpp"
#include "performance.hpp"
#include "shared_counter.hpp"
#include "shared_value.hpp"
#include "module/module.hpp"
#include "libraryoptions.hpp"
#include "platform.hpp"

#include <spdlog/spdlog.h>

#include <map>

static std::size_t handle_threads_options(const Options & options)
{
	auto threads = options.get("threads", int(std::thread::hardware_concurrency()));
	/*
	Avoid both having less than one thread,
	but also to have no more than file descriptors available.
	*/
	return (std::min)(std::size_t((std::max)(threads, 1)), platform::fd::max()-1);
}

void ErrorStats::print() const
{
	if (errors[ErrorStats::ERROR_COMPRESSION] > 0)
		spdlog::warn("Compression: {}", errors[ErrorStats::ERROR_COMPRESSION]);
	if (errors[ErrorStats::ERROR_TYPE] > 0)
		spdlog::warn("Type: {}", errors[ErrorStats::ERROR_TYPE]);
	if (errors[ErrorStats::ERROR_PARSE] > 0)
		spdlog::warn("Parse: {}", errors[ErrorStats::ERROR_PARSE]);
	if (errors[ErrorStats::ERROR_EMPTY_CHUNKS] > 0)
		spdlog::warn("Empty Chunks: {}", errors[ErrorStats::ERROR_EMPTY_CHUNKS]);
	if (errors[ErrorStats::ERROR_EMPTY_REGIONS] > 0)
		spdlog::warn("Empty Regions: {}", errors[ErrorStats::ERROR_EMPTY_REGIONS]);
	if (errors[ErrorStats::ERROR_LONELY_REGIONS] > 0)
		spdlog::warn("Lonely Regions: {}", errors[ErrorStats::ERROR_LONELY_REGIONS]);
	if (errors[ErrorStats::ERROR_LONELY_CHUNKS] > 0)
		spdlog::warn("Lonely Chunks: {}", errors[ErrorStats::ERROR_LONELY_CHUNKS]);
}

std::size_t PerfStats::createPerfValue(const std::string & name)
{
	auto id = perfValues.size();
	perfValues.emplace_back(std::make_shared<PerfValue>(name));
	return id;
}

void PerfStats::print() const
{
#ifdef PERF_DEBUG
	for (const auto & perf : perfValues)
		if (perf->value > 0)
			spdlog::info("{:s}: {:.6f}s", perf->name, perf->value / 1000);
#endif

	errors.print();

	std::map<std::string, int> counters;
	for (const auto & str : errorString.get())
	{
		auto it = counters.find(str);
		if (it == counters.end())
			counters.emplace(str, 1);
		else
			++it->second;
	}

	for (const auto & p : counters) {
		spdlog::error("{}: {}", p.first, p.second);
	}
}

struct RenderModule
{
	Module mod;
	std::shared_ptr<PassBuilder> builder;
};

WorkerBase::WorkerBase(std::atomic_bool & _run, const Options & options) :
	_valid(false),
	run(_run),
	use_lonely(!options.get<bool>("nolonely", false)),
	pool(handle_threads_options(options), 0),
	total_chunks(0),
	total_regions(0)
{
	settings = std::make_shared<RenderSettings>();

	// Load block color
	if (options.has("colors"))
		settings->colors.read(options.get<std::string>("colors"));
	else
		settings->colors.read("blockcolor.conf");

	if (!settings->colors.hasColors())
	{
		spdlog::warn("Using default colors");
		if (!settings->colors.read())
		{
			spdlog::error("Internal error: No colors found, contact developer");
			return;
		}
	}

	// Set render type
	{
		auto type = options.get<std::string>("imageType");
		if (type == "chunk")
			settings->mode = Render::Mode::CHUNK;
		else if (type == "region" || type == "map")
			settings->mode = Render::Mode::REGION;
		else if (type == "image")
			settings->mode = Render::Mode::IMAGE;
#ifdef ENABLE_WEBVIEW
		else if (type == "web" || type == "webview")
			settings->mode = Render::Mode::WEBVIEW;
#endif
		else if (type == "image_direct" || type == "direct")
			settings->mode = Render::Mode::IMAGE_DIRECT;
		else if (type == "tiny_chunk")
			settings->mode = Render::Mode::CHUNK_TINY;
		else if (type == "tiny_region")
			settings->mode = Render::Mode::REGION_TINY;
		else
			settings->mode = Render::Mode::DEFAULT;
	}

	BlockPassFunction blockPass;

	// Load custom pipeline library for rendering
	if (options.has("pipeline"))
	{
		mod = std::make_shared<RenderModule>();
		auto libopt = options.get<LibraryOptions>("pipeline");
		if (!mod->mod.load(libopt.library))
		{
			spdlog::error("Library error: {:s}", mod->mod.getError());
			return;
		}
		else
		{
			auto loadArguments = mod->mod.getFunction<bool(void**)>("module_loadArguments");
			if (loadArguments)
			{
				std::vector<void*> arguments;
				arguments.resize(libopt.arguments.size()+1);
				for (std::size_t i = 0; i < libopt.arguments.size(); ++i)
					arguments[i] = static_cast<void *>(libopt.arguments[i].data());
				arguments[libopt.arguments.size()] = nullptr;
				if (!loadArguments(arguments.data()))
				{
					spdlog::error("Library error: Unable to load arguments");
					return;
				}
			}
			auto version = mod->mod.version();
			spdlog::info("Loaded library {:s} version {:d}", libopt.library, version);
			mod->builder = mod->mod.createIntance<PassBuilder>("RenderPassBuilder");
			blockPass = mod->builder->build();
		}
	}
	// Default rendering
	else
	{
		// Get internal values
		auto heightline = options.get<int>("heightline", -1);
		auto slice = options.get<int>("slice", std::numeric_limits<int>::min());

		auto blendStr = options.get<std::string>("blend", "legacy");
		// TODO: Move this elsewhere
		std::unordered_map<std::string, BlockPass::Blend::Mode> blendModes{
			{"legacy", BlockPass::Blend::Mode::LEGACY},
			{"normal", BlockPass::Blend::Mode::NORMAL},
			{"multiply", BlockPass::Blend::Mode::MULTIPLY},
			{"screen", BlockPass::Blend::Mode::SCREEN},
			{"overlay", BlockPass::Blend::Mode::OVERLAY},
			{"darken", BlockPass::Blend::Mode::DARKEN},
			{"lighten", BlockPass::Blend::Mode::LIGHTEN},
			{"color_dodge", BlockPass::Blend::Mode::COLOR_DODGE},
			{"color_burn", BlockPass::Blend::Mode::COLOR_BURN},
			{"hard_light", BlockPass::Blend::Mode::HARD_LIGHT},
			{"soft_light", BlockPass::Blend::Mode::SOFT_LIGHT},
			{"difference", BlockPass::Blend::Mode::DIFFERENCE_},
			{"exclusion", BlockPass::Blend::Mode::EXCLUSION},
			{"hue", BlockPass::Blend::Mode::HUE},
			{"saturation", BlockPass::Blend::Mode::SATURATION},
			{"color", BlockPass::Blend::Mode::COLOR},
			{"luminosity", BlockPass::Blend::Mode::LUMINOSITY},
		};
		BlockPass::Blend::Mode blend = BlockPass::Blend::Mode::LEGACY;
		if (blendModes.find(blendStr) == blendModes.end())
			spdlog::warn("Invalid blend mode '{:s}', using default", blendStr);
		else
			blend = blendModes[blendStr];

		// TODO: Move this elsewhere
		BlockPassBuilder builder;
		{
			using namespace BlockPass;
			builder.add("default", Default().build());
			builder.add("opaque", Opaque().build());
			builder.add("heightmap", Heightmap().build());
			builder.add("gray", Gray().build());
			builder.add("color", Color().build());
			builder.add("heightline", Heightline(heightline).build());
			builder.add("night", Night().build());
			builder.add("slice", Slice(slice).build());
			builder.add("cave", Cave().build());
			builder.add("blend", Blend(blend).build());
		}

		std::vector<std::string> passes = { "default" };
		// Slice
		if (slice > std::numeric_limits<int>::min())
			passes.push_back("slice");
		// Cave
		if (options.get<bool>("cave", false))
			passes.push_back("cave");
		// Blend or opaque
		passes.push_back(options.get<bool>("opaque", false) ? "opaque" : "blend");
		// Color mode
		{
			auto mode = options.get<std::string>("mode", "");
			if (mode == "gray")
				passes.push_back("gray");
			else if (mode == "color")
				passes.push_back("color");
			else
			{
				// Height gradient
				if (options.get<bool>("heightgradient", false))
					passes.push_back("heightmap");
				// Height line
				if (heightline > 0 && heightline < 256)
					passes.push_back("heightline");
				// Night
				if (options.get<bool>("night", false))
					passes.push_back("night");
			}
		}

		blockPass = builder.generate(passes);
	}

	chunkPass = ChunkPassFactory::create(settings, blockPass);
	regionPass = RegionPassFactory::create(settings);
	worldPass = WorldPassFactory::create(settings);

	_valid = true;
}

void WorkerBase::eventTotalChunks(std::function<void(int)> && func)
{
	func_totalChunks.add(std::move(func));
}

void WorkerBase::eventFinishedChunk(std::function<void(int)> && func)
{
	func_finishedChunk.add(std::move(func));
}

void WorkerBase::eventFinishedChunk(std::function<void()> && func)
{
	func_finishedChunks.add(std::move(func));
}

void WorkerBase::eventTotalRender(std::function<void(int)> && func)
{
	func_totalRender.add(std::move(func));
}

void WorkerBase::eventFinishedRender(std::function<void(int)> && func)
{
	func_finishedRender.add(std::move(func));
}

void WorkerBase::eventFinishedRender(std::function<void()> && func)
{
	func_finishedRenders.add(std::move(func));
}

void WorkerBase::eventTotalExtra(std::function<void(int)> && func)
{
	func_totalExtra.add(std::move(func));
}

void WorkerBase::eventFinishedExtra(std::function<void(int)> && func)
{
	func_finishedExtra.add(std::move(func));
}

void WorkerBase::eventFinishedExtra(std::function<void()> && func)
{
	func_finishedExtras.add(std::move(func));
}

