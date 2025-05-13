
#include "gui.hpp"
#include "pixelmap.hpp"
#include "minecraft.hpp"
#include "timer.hpp"
#include "log.hpp"
#include "libraryoptions.hpp"
#include "blockcolor.hpp"
#include "version.hpp"

#include "imgui/imgui_custom.h"

#include <nfd.hpp>

#include <vector>
#include <string>
#include <algorithm>
#include <tuple>
#include <chrono>
#include <thread>
#include <future>
#include <mutex>
#include <queue>
#include <iostream>
#include <filesystem>

using namespace std::chrono_literals;

/**
 * @brief Check if future is available
 * Verify validity and checks if correct.
 * @tparam T Future return type
 * @param v Future
 * @return true Is available
 * @return false Is not available
 */
template<typename T>
bool isAvailable(const std::future<T> & v)
{
	return v.valid() && v.wait_for(std::chrono::duration<double, std::milli>(0)) == std::future_status::ready;
}

class TimeUpdate
{
	typedef std::chrono::steady_clock Clock;
public:
	explicit TimeUpdate(const Clock::duration & duration)
		: clock(Clock::time_point::min()), duration(duration)
	{}
	inline void reset()
	{
		clock = Clock::now() + duration;
		running = true;
	}
	inline bool shouldUpdate()
	{
		return (running && Clock::now() >= clock) ? !(running = false) : false;
	}
private:
	Clock::time_point clock;
	Clock::duration duration;
	bool running = false;
};

int main(int, char**)
{
	Log::InitFile(spdlog::level::info, "log.txt");

	GUI gui;
	std::string title = fmt::format("PixelMap {:s}", Version::version);
	gui.create(title, 400, 340);
	gui.set_fps(20);

	PixelMap pm;
	Options options;
	std::atomic_int finishedChunks(0);
	std::atomic_int finishedRender(0);
	std::atomic_int totalChunks(0);
	std::atomic_int totalRender(0);

	pm.eventTotalChunks([&totalChunks, &gui](int a) {
		totalChunks = a;
		gui.refresh();
	});
	pm.eventTotalRender([&totalRender, &gui](int a) {
		totalRender = a;
		gui.refresh();
	});
	pm.eventFinishedChunk([&finishedChunks, &gui](int a) {
		finishedChunks += a;
		gui.refresh();
	});
	pm.eventFinishedRender([&finishedRender, &gui](int a) {
		finishedRender += a;
		gui.refresh();
	});

	Timer<> timer;

	std::future<std::shared_ptr<Minecraft::WorldInfo>> worldInfoWorker;

	bool use_custom_path = false;
	TimeUpdate custom_path_timer(500ms);
	std::string custom_path = "";

	std::vector<std::array<std::string, 2UL>> libraryTypes;
	#if defined(_WIN32)
	libraryTypes = {{"DLL Library", "dll"}};
	#elif defined(__APPLE__)
	libraryTypes = {
		{"Dynamic Library", "dylib"},
		{"Bundle", "bundle"},
		{"Unix Library", "so"}
	};
	#elif defined(__linux__)
	libraryTypes = {{"Unix Library", "so"}};
	#endif
	std::string libraryPath;
	struct StringItem
	{
		int id;
		std::string str;
	};
	std::vector<StringItem> libraryArgs;
	std::string newArg;

	std::vector<std::string> types = {"Default", "Gray map", "Color map"};
	std::vector<std::string> types_data = {"default", "gray", "color"};
	std::size_t type_selected = 0;
	bool slice_enabled = false;
	int slice = 0;
	bool night = false, cave_mode = false, no_lonely = false, height_gradient = false, opaque = false;
	std::vector<std::string> output_types = {"Image", "Map"};
	std::vector<std::string> output_types_data = {"image", "map"};
	std::size_t output_type_selected = 0;
	std::string outputPath = "image.png";
	int workers = std::thread::hardware_concurrency();
	bool auto_close = false;
	std::string colors = "blockcolor.conf";
	TimeUpdate updateDefaultColors(1s);
	std::future<bool> browse;

	auto minecraft_paths = Minecraft::getDefaultPaths();
	std::vector<std::string> minecraft_names;
	minecraft_names.reserve(minecraft_paths.size());
	// Note: Maybe want natural sorting
	std::sort(minecraft_paths.begin(), minecraft_paths.end());
	std::transform(minecraft_paths.begin(), minecraft_paths.end(), std::back_inserter(minecraft_names),
		[](const std::string & path) { return std::filesystem::path(path).filename().string(); });
	std::size_t minecraft_path_selected = 0;
	auto worldInfo = std::make_shared<Minecraft::WorldInfo>();
	Minecraft::WorldInfo::DimensionInfo dimInfo;
	std::vector<std::string> dimensions, dimensions_paths;
	std::size_t dimension_selected = 0;

	if (!minecraft_paths.empty())
	{
		worldInfoWorker = std::async(std::launch::async, [&gui](const std::string & path) {
			gui.refresh();
			return Minecraft::getWorldInfo(path);
		}, minecraft_paths[minecraft_path_selected]);
	}

	while (gui.alive())
	{
		if (isAvailable(worldInfoWorker))
		{
			worldInfo = worldInfoWorker.get();
			dimensions.clear();
			dimension_selected = 0;
			std::transform(worldInfo->dimensions.begin(), worldInfo->dimensions.end(), std::back_inserter(dimensions), [](const auto & dim) { return dim.name; });
			gui.refresh();
		}
		if (!gui.begin())
			continue;

		#ifdef IMGUI_HAS_VIEWPORT
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->GetWorkPos());
		ImGui::SetNextWindowSize(viewport->GetWorkSize());
		ImGui::SetNextWindowViewport(viewport->ID);
		#else 
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
		#endif
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::Begin("PixelMap", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoResize);
		{
			if (ImGui::BeginTabBar("options"))
			{
				if (ImGui::BeginTabItem("Input"))
				{
					ImGui::BeginDisabled(worldInfoWorker.valid());
					if (ImGui::BeginTabBar("path"))
					{
						if (ImGui::BeginTabItem("Minecraft path"))
						{
							auto prev = minecraft_path_selected;
							if (use_custom_path)
							{
								prev = -1;
								use_custom_path = false;
							}
							ImGui::PushItemWidth(-1);
							GUI::Combo("##minecraft_path", &minecraft_path_selected, minecraft_names);
							ImGui::PopItemWidth();
							ImGui::EndTabItem();
							if (prev != minecraft_path_selected && !minecraft_paths.empty())
							{
								worldInfoWorker = std::async(std::launch::async, [&gui](const std::string & path) {
									auto world = Minecraft::getWorldInfo(path);
									gui.refresh();
									return world;
								}, minecraft_paths[minecraft_path_selected]);
								gui.refresh();
							}
						}
						if (ImGui::BeginTabItem("Custom path"))
						{
							auto prev = custom_path;
							if (!use_custom_path)
							{
								prev.clear();
								use_custom_path = true;
							}
							ImGui::PushItemWidth(-60);
							browse = GUI::BrowseFolder("##custom_path", custom_path, custom_path);
							ImGui::PopItemWidth();
							if (prev != custom_path)
							{
								custom_path_timer.reset();
							}
							else if (custom_path_timer.shouldUpdate())
							{
								worldInfoWorker = std::async(std::launch::async, [&gui](const std::string & path) {
									auto world = Minecraft::getWorldInfo(path);
									gui.refresh();
									return world;
								}, custom_path);
								gui.refresh();
							}
							ImGui::EndTabItem();
						}
						ImGui::EndTabBar();
					}

					ImGui::PushItemWidth(-70);
					GUI::Combo("Dimension", &dimension_selected, dimensions);
					ImGui::PopItemWidth();

					ImGui::EndDisabled();

					ImGui::BeginGroupPanel("Info", ImVec2(-1, 0));
					{
						if (!worldInfo->name.empty())
							ImGui::Text("Name: %s", worldInfo->name.c_str());
						if (worldInfo->game == Minecraft::Game::JAVA_EDITION)
							ImGui::Text("Java");
						else if (worldInfo->game == Minecraft::Game::BEDROCK_EDITION)
							ImGui::Text("Bedrock");
						if (!worldInfo->dimensions.empty())
							ImGui::Text("Chunk count: %lu", worldInfo->dimensions[dimension_selected].amount_chunks);
						ImGui::LabelText("###seed", "Seed: %ld", worldInfo->seed);
						if (ImGui::IsItemClicked())
						{
							auto str = std::to_string(worldInfo->seed);
							ImGui::SetClipboardText(str.c_str());
						}
						ImGui::Text("Days: %ld", worldInfo->ticks / 24000);
						auto time = worldInfo->time % 24000;
						// https://minecraft.wiki/w/Daylight_cycle#24-hour_Minecraft_day
						ImGui::Text("Day time: %s",
							time < 12000 ? "Day" :
							time < 13000 ? "Sunset" :
							time < 23000 ? "Night" : "Sunrise");
						if (!worldInfo->minecraftVersion.empty())
							ImGui::Text("Version: %s", worldInfo->minecraftVersion.c_str());
					}
					ImGui::EndGroupPanel();
					
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Output"))
				{
					ImGui::PushItemWidth(-1);
					GUI::Combo("##type", &type_selected, types);
					ImGui::PopItemWidth();

					ImGui::BeginGroupPanel("Slice", ImVec2(-1, 0));
					{
						ImGui::Checkbox("###slice_on", &slice_enabled); ImGui::SameLine();
						ImGui::BeginDisabled(!slice_enabled);
						ImGui::PushItemWidth(-1);
						ImGui::InputInt("###slice", &slice, 1, 8);
						ImGui::PopItemWidth();
						ImGui::Dummy(ImVec2(0, 4));
						ImGui::EndDisabled();
					}
					ImGui::EndGroupPanel();

					ImGui::BeginGroupPanel("Options", ImVec2(-1, 0));
					{
						ImGui::Checkbox("Night", &night); ImGui::SameLine();
						if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
							ImGui::SetTooltip("Dim down blocks, lighten up light sources");
						else if (ImGui::IsItemHovered())
							gui.refresh();
						ImGui::Checkbox("Cave mode", &cave_mode); ImGui::SameLine();
						if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
							ImGui::SetTooltip("Skip first top solid blocks");
						else if (ImGui::IsItemHovered())
							gui.refresh();
						ImGui::Checkbox("Keep lonely", &no_lonely);
						if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
							ImGui::SetTooltip("Keeps lonely chunks and regions");
						else if (ImGui::IsItemHovered())
							gui.refresh();
						ImGui::Checkbox("Height gradient", &height_gradient); ImGui::SameLine();
						if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
							ImGui::SetTooltip("Gradient dark to light from bottom to top");
						else if (ImGui::IsItemHovered())
							gui.refresh();
						ImGui::Checkbox("Opaque", &opaque);
						if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip))
							ImGui::SetTooltip("Disable transclucent blocks");
						else if (ImGui::IsItemHovered())
							gui.refresh();
						ImGui::Dummy(ImVec2(0, 4));
					}
					ImGui::EndGroupPanel();

					ImGui::BeginGroupPanel("Output", ImVec2(-1, 0));
					{
						ImGui::PushItemWidth(-1);
						GUI::Combo("##output_type", &output_type_selected, output_types);
						ImGui::PopItemWidth();
						if (output_type_selected == 0)
						{
							browse = GUI::BrowseSave("###output", outputPath, { {"Image", "png"} });
						}
						else
						{
							browse = GUI::BrowseFolder("###output", outputPath, outputPath);
						}
						ImGui::Dummy(ImVec2(0, 4));
					}
					ImGui::EndGroupPanel();

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Advanced"))
				{
					ImGui::PushItemWidth(100);
					ImGui::InputInt("Workers", &workers);
					if (workers < 0) workers = 0;
					auto rev = fmt::format("Revision: {:s}", Version::version_revision);
					ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - ImGui::CalcTextSize(rev.c_str()).x);
					ImGui::Text("%s", rev.c_str());
					if (ImGui::IsItemClicked())
					{
						ImGui::SetClipboardText(Version::version_revision);
					}
					ImGui::PopItemWidth();

					ImGui::PushItemWidth(-54);
					// Disabled for now, but only because it is highly experimental
					ImGui::BeginDisabled();
					ImGui::BeginGroupPanel("Module", ImVec2(-1, 0));
					{
						browse = GUI::BrowseLoad("##library", libraryPath, libraryTypes);
						int index = 0;
						for (auto & arg : libraryArgs)
						{
							auto name = fmt::format("arg {:d}###arg{:d}", index, arg.id);
							ImGui::InputText(name.c_str(), arg.str);
							++index;
						}
						auto nextId = libraryArgs.empty() ? 0 : libraryArgs.back().id + 1;
						if (libraryArgs.size() < 4)
						{
							auto name = fmt::format("arg {:d}###arg{:d}", index, nextId);
							ImGui::InputText(name.c_str(), newArg);
						}
						if (newArg[0] != '\0')
						{
							libraryArgs.emplace_back(StringItem{nextId, newArg});
							newArg = "";
						}
						libraryArgs.erase(std::remove_if(libraryArgs.begin(), libraryArgs.end(), [](const StringItem & item) { return item.str[0] == '\0'; }), libraryArgs.end());
						ImGui::Dummy(ImVec2(0, 4));
					}
					ImGui::EndGroupPanel();
					ImGui::EndDisabled();

					ImGui::Dummy(ImVec2(4, 0));

					ImGui::BeginGroupPanel("Action", ImVec2(-1, 0));
					{
						ImGui::Checkbox("Auto-close", &auto_close);
						ImGui::Dummy(ImVec2(0, 4));
					}
					ImGui::EndGroupPanel();

					ImGui::BeginGroupPanel("Colors", ImVec2(-1, 0));
					{
						ImGui::PushItemWidth(-64);
						browse = GUI::BrowseLoad("##colors", colors, { { "BlockColor Files", "conf" } });
						ImGui::PopItemWidth();
						if (ImGui::Button("Create"))
						{
							if (!BlockColor::writeDefault(colors))
							{
								updateDefaultColors.reset();
								ImGui::OpenPopup("Failed");
							}
						}
						ImGui::Dummy(ImVec2(0, 4));
					}
					ImGui::SetNextWindowSize(ImVec2(0, 0));
					ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(.5f, .5f));
					if (ImGui::BeginPopupModal("Failed", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
					{
						ImGui::Text("Failed to create default block color file.");
						if (updateDefaultColors.shouldUpdate())
						{
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}
					ImGui::EndGroupPanel();

					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::Dummy(ImVec2(0, 10));
			ImGui::BeginDisabled(worldInfo->dimensions.empty());
			if (ImGui::Button("Render", ImVec2(-1, 0)))
			{
				ImGui::OpenPopup("rendering");
				totalChunks = 0;
				finishedChunks = 0;
				totalRender = 0;
				finishedRender = 0;
				options.clear();
				options.set("threads", workers);
				if (!libraryPath.empty())
				{
					LibraryOptions libopt;
					libopt.library = libraryPath;
					for (auto & at : libraryArgs)
						libopt.arguments.emplace_back(at.str);
					options.set("pipeline", libopt);
				}
				options.set("imageType", output_types_data[output_type_selected]);
				options.set("mode", types_data[type_selected]);
				if (!colors.empty())
					options.set("colors", colors);
				if (slice_enabled)
					options.set("slice", slice);
				if (night)
					options.set("night", true);
				if (opaque)
					options.set("opaque", true);
				if (cave_mode)
					options.set("cave", true);
				if (no_lonely)
					options.set("nolonely", true);
				if (height_gradient)
					options.set("heightgradient", true);
				pm.set(options);
				timer.start();
				auto path = (!use_custom_path) ? minecraft_paths[minecraft_path_selected] : custom_path;
				pm.start(path, outputPath, worldInfo->dimensions[dimension_selected].dimension);
			}
			ImGui::EndDisabled();
			ImGui::SetNextWindowSize(ImVec2(220, 160));
			ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(.5f, .5f));
			if (ImGui::BeginPopupModal("rendering", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
			{
				ImGui::Text("Rendering...");
				auto chunksProgress = totalChunks ? (float)finishedChunks / totalChunks : 0.0f;
				auto renderProgress = totalRender ? (float)finishedRender / totalRender : 0.0f;
				ImGui::ProgressBar(chunksProgress, ImVec2(200, 20));
				ImGui::ProgressBar(renderProgress, ImVec2(200, 20));
				ImGui::Text("%d/%d", (int)finishedChunks, (int)totalChunks); ImGui::SameLine(100);
				ImGui::Text("%d/%d", (int)finishedRender, (int)totalRender);
				auto totalProgress = (chunksProgress + renderProgress) / 2.0f;
				if (totalProgress > 0 && timer.elapsed() > 0)
				{
					auto elapsed = timer.elapsed();
					int seconds = elapsed / totalProgress - elapsed;
					auto hours = seconds / 3600;
					seconds %= 3600;
					auto minutes = seconds / 60;
					seconds %= 60;
					if (hours > 0)
						ImGui::Text("%02d:%02d:%02d", hours, minutes, seconds);
					else
						ImGui::Text("%02d:%02d", minutes, seconds);
				}
				else
				{
					ImGui::Text("--:--");
				}
				
				if (ImGui::Button("Abort"))
					pm.stop();
				if (pm.done())
				{
					ImGui::CloseCurrentPopup();
					if (auto_close)
						gui.close();
				}
				ImGui::EndPopup();
			}
		}
		ImGui::End();
		ImGui::PopStyleVar();
		gui.end();
		if (browse.valid())
			browse.wait();
	}

	gui.destroy();
	pm.wait();
	return 0;
}
