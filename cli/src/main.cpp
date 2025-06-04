
#include "timer.hpp"
#include "pixelmap.hpp"
#include "programoptions.hpp"
#include "console.hpp"
#include "events.hpp"
#include "log.hpp"
#include "libraryoptions.hpp"
#include "blockcolor.hpp"
#include "version.hpp"
#ifdef ENABLE_PROFILER
#include "profiler.hpp"
#endif

#include <spdlog/spdlog.h>

#include <iostream>
#include <unordered_set>
#include <chrono>
#include <thread>
#include <cstdlib>

// Pass arguments to options
inline Options & operator<<(Options & options, const ProgramOptions & arguments)
{
	// These are the only parameters that will be handled specifically
	std::unordered_set<std::string> passthrough = {
		"threads",
		"dimension",
		"colors",
		"mode",
		"blend",
		"slice",
		"heightline",
		"opaque",
		"heightgradient",
		"night",
		"imageType",
		"cave",
		"nolonely"
	};
	for (auto & vars : arguments.getParameters())
	{
		if (passthrough.find(vars.first) == passthrough.end())
			continue;

		Any data;
		if (vars.second.empty())
			data = true;
		else if (vars.second.size() > 1)
			data = vars.second;
		else
			data = vars.second[0];
		options.set(vars.first, data);
	}
	return options;
}

int main(int argc, const char * argv[])
{
	using namespace std::chrono_literals;
	// Prepare arguments
	ProgramOptions arguments(argc, reinterpret_cast<const char **>(argv), "pixelmapcli [options] <input> <output>");
	arguments.setHeader("Top-down pixel-perfect Minecraft world mapper.");
	arguments.setFooter("Licensed with GPLv3");

	arguments.addParamType<int>("threads", 't', "threads", 1);
	arguments.addParamType<int>("dimension", 'd', 1);
	arguments.addParamType<std::string>("colors", 'p', 1);
	arguments.addParamType<std::string>("mode", 'm', 1); // default, gray, color
	arguments.addParamType<std::string>("blend", "blend", 1);
	arguments.addParamType<int>("slice", "slice", 1);
	arguments.addParamType<int>("heightline", "heightline", 1);
	arguments.addParam("opaque", "opaque");
	arguments.addParam("heightgradient", 'g', "gradient");
	arguments.addParam("night", 'n', "night");
	arguments.addParamType<std::string>("imageType", 'r', "render", 1);
	arguments.addParam("cave", 'c', "cave");
	arguments.addParamType<std::string>("pipeline", "lib", 1);
	arguments.addParamType<std::string>("pipelineArgs", 'a', "arg", 1);
	arguments.addParamType<std::string>("createColor", "createcolor", 1);
	arguments.addParam("nolonely", "no-lonely");
	arguments.addParam("verbal", 'v');
	arguments.addParam("quiet", 'q');
	arguments.addParam("nocolor", "no-color");
	arguments.addParam("help", 'h', "help");
	arguments.addParam("version", 'v', "version");

	// Prepare help section
	arguments.addHelp("threads", "The amount of threads to create. Default is amount of cores.");
	arguments.addHelp("dimension", "The dimension to render.");
	arguments.addHelp("colors", "The block color file.");
	arguments.addHelp("mode", "The mode to render in: default, gray, color.");
	arguments.addHelp("blend", "When not opaque, pick a blend mode.");
	arguments.addHelp("slice", "Slice from height.");
	arguments.addHelp("heightline", "Put height line on every n.");
	arguments.addHelp("opaque", "Render blocks as opaque.");
	arguments.addHelp("heightgradient", "Put a darker gradient on the blocks depending on the height");
	arguments.addHelp("night", "Render as if night.");
	arguments.addHelp("imageType", "Specify output mode: chunk, map, image(default)");
	arguments.addHelp("cave", "Render next cave.");
	arguments.addHelp("pipeline", "Set library.");
	arguments.addHelp("pipelineArgs", "Set library parameters.");
	arguments.addHelp("createColor", "Create block color file from default.");
	arguments.addHelp("nolonely", "Disable lonely checking.");
	arguments.addHelp("verbal", "Display more to the user.");
	arguments.addHelp("quiet", "Silence all output.");
	arguments.addHelp("nocolor", "Turn off the console color.");
	arguments.addHelp("help", "This help text.");
	arguments.addHelp("version", "The version of the program.");

	bool no_color = std::getenv("NO_COLOR") != nullptr;

	// Parse the arguments
	if (!arguments.parse())
	{
		for (const auto & err : arguments.getErrors())
			std::cerr << err << std::endl;
		return 1;
	}

	const auto & params = arguments.getParameters();

	if (params.find("nocolor") != params.end())
		no_color = true;

	Log::InitConsole(spdlog::level::info, !no_color);

	if (params.find("help") != params.end())
	{
		arguments.printHelp();
		return 0;
	}

	if (params.find("version") != params.end())
	{
		std::cout << Version::version << "-" << Version::version_revision << std::endl;
		return 0;
	}

	if (params.find("createColor") != params.end())
	{
		auto success = BlockColor::writeDefault(params.find("createColor")->second[0].get<std::string>());
		return success ? 0 : 1;
	}

	const auto & args = arguments.getArguments();
	if (args.size() != 2)
	{
		std::cerr << "Requires one path and one output" << std::endl;
		return 1;
	}

	PixelMap pixelmap;
	// Prepare options for PixelMap
	Options options;
	options << arguments;

	// Library handling
	if (params.find("pipeline") != params.end())
	{
		LibraryOptions libopt;
		auto it = params.find("pipeline");
		libopt.library = it->second.back().get<std::string>();
		if (params.find("pipelineArgs") != params.end())
			for (auto & at : params.find("pipelineArgs")->second)
				libopt.arguments.emplace_back(at.get<std::string>());
		options.set("pipeline", libopt);
	}

	pixelmap.set(options);

	bool verbal = params.find("verbal") != params.end();
	bool quiet = params.find("quiet") != params.end();

	Console console;
	std::atomic_int finishedChunks(0);
	std::atomic_int finishedRender(0);
	std::atomic_int totalChunks(0);
	std::atomic_int totalRender(0);

	pixelmap.eventTotalChunks([&totalChunks](int a) { totalChunks = a; });
	pixelmap.eventTotalRender([&totalRender](int a) { totalRender = a; });
	pixelmap.eventFinishedChunk([&finishedChunks](int a) { finishedChunks += a; });
	pixelmap.eventFinishedRender([&finishedRender](int a) { finishedRender += a; });

	bool aborted = false;

	// Set events handling
	Events::registerInterrupt([&pixelmap, &aborted]()
		{
			aborted = true;
			pixelmap.stop();
		});

	auto dimension = (params.find("dimension") != params.end()) ? params.find("dimension")->second.back().get<int>() : 0;

#ifdef ENABLE_PROFILER
	MemoryProfiler profile;
#endif

	// Start timer
	Timer<> timer;
	if (!quiet)
		timer.start();

	// Send in the path
	pixelmap.start(args[0], args[1], dimension);

	// Wait until done
	while (!pixelmap.done())
	{
		if (!aborted && !quiet && timer.elapsed() > 1)
			console.progress(totalChunks + totalRender, finishedChunks + finishedRender);
		std::this_thread::sleep_for(100ms);
	}
	if (aborted)
		return 1;

	if (!quiet && timer.elapsed() > 1)
	{
		console.progress(totalChunks + totalRender, finishedChunks + finishedRender);
	}

	if (!quiet)
	{
		// Get time elapsed
		std::cout << "Total time: " << timer.elapsed() << std::endl;
		if (verbal)
		{
			std::cout << "Chunks: " << finishedChunks << "/" << totalChunks << std::endl;
			std::cout << "Render: " << finishedRender << "/" << totalRender << std::endl;
		}
#ifdef ENABLE_PROFILER
		profile.print();
#endif
	}
	return 0;
}
