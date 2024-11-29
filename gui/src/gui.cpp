#include "gui.hpp"

#include "icon.hpp"
#include "resource_DroidSans_ttf.hpp"

#include "imgui/imgui_custom.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#include <backends/imgui_impl_opengl3.h>
#include <spdlog/spdlog.h>
#include <nfd.hpp>

#include <SDL.h>
#include <SDL_opengl.h>

#include <algorithm>
#include <iterator>
#include <future>
#include <iostream>

struct Data
{
	SDL_Window * window;
	SDL_GLContext gl_context;
	ImVec4 clear_color = {0.45f, 0.55f, 0.60f, 1.0f};
};

void GUI::create(const std::string & title, int w, int h)
{
	data = std::make_shared<Data>();
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		spdlog::error("Error: {:s}", SDL_GetError());
		return;
	}

	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

	// Create window with graphics context
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	data->window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, window_flags);
	data->gl_context = SDL_GL_CreateContext(data->window);
	SDL_GL_MakeCurrent(data->window, data->gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync

	auto icon = SDL_CreateRGBSurfaceFrom(
		const_cast<uint8_t*>(pixelmapim),
		PIXELMAPIM_WIDTH, PIXELMAPIM_HEIGHT,
		PIXELMAPIM_DEPTH * PIXELMAPIM_CHANNELS,
		PIXELMAPIM_WIDTH * PIXELMAPIM_CHANNELS,
		0x0000FF, 0x00FF00, 0xFF0000, 0x000000
	);
	if (icon)
		SDL_SetWindowIcon(data->window, icon);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO & io = ImGui::GetIO();
	ImFontConfig font_cfg;
	font_cfg.FontDataOwnedByAtlas = false;
	io.Fonts->AddFontFromMemoryTTF(
			resource_DroidSans_ttf_data, resource_DroidSans_ttf_size,
			16.f, &font_cfg);
	
	ImGui_ImplSDL2_InitForOpenGL(data->window, data->gl_context);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Draw order
	frames.emplace_back(Framed{ ImGui_ImplOpenGL3_NewFrame, [this, io](){
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(data->clear_color.x * data->clear_color.w,
			data->clear_color.y * data->clear_color.w,
			data->clear_color.z * data->clear_color.w,
			data->clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	} });
	frames.emplace_back(Framed{ [](){ ImGui_ImplSDL2_NewFrame(); }, [this](){ SDL_GL_SwapWindow(data->window); } });
	frames.emplace_back(Framed{ ImGui::NewFrame, ImGui::Render });

	NFD::Init();
}

void GUI::destroy()
{
	NFD::Quit();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(data->gl_context);
	SDL_DestroyWindow(data->window);
	SDL_Quit();
	data.reset();
}

void GUI::begin()
{
	SDL_Event event;
	while(SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (event.type == SDL_QUIT)
			run = false;
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(data->window))
			run = false;
	}
	std::for_each(frames.begin(), frames.end(), [](auto & frame) { frame.begin(); });
}

void GUI::end()
{
	std::for_each(frames.rbegin(), frames.rend(), [](auto & frame) { frame.end(); });
}

/**
 * Helper functions
 */

bool GUI::Combo(const char * label, int * current_item, const std::vector<std::string> & items)
{
	const char * preview_value = nullptr;
	if (!items.empty())
		preview_value = items[*current_item].c_str();
	else
		ImGui::BeginDisabled();
	if (ImGui::BeginCombo(label, preview_value))
	{
		for (int i = 0; i < items.size(); ++i)
		{
			if (ImGui::Selectable(items[i].c_str(), i == *current_item))
				*current_item = i;
		}
		ImGui::EndCombo();
		return true;
	}
	if (items.empty())
		ImGui::EndDisabled();
	return false;
}

std::future<bool> GUI::BrowseLoad(const char * label,
	std::string & outPath,
	const std::vector<std::array<std::string, 2>> & items,
	const std::string & defaultPath)
{
	std::future<bool> ret;
	ImGui::BeginGroup();
	ImGui::InputText(label, outPath); ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		ret = std::async(std::launch::async, [&outPath, &items, &defaultPath]() {
			return OpenDialog(outPath, items, defaultPath);
		});
	}
	ImGui::EndGroup();
	return ret;
}

std::future<bool> GUI::BrowseSave(const char * label,
	std::string & outPath,
	const std::vector<std::array<std::string, 2>> & items,
	const std::string & defaultPath,
	const std::string & defaultName)
{
	std::future<bool> ret;
	ImGui::BeginGroup();
	ImGui::InputText(label, outPath); ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		ret = std::async(std::launch::async, [&outPath, &items, &defaultPath, &defaultName]() {
			return SaveDialog(outPath, items, defaultPath, defaultName);
		});
	}
	ImGui::EndGroup();
	return ret;
}

std::future<bool> GUI::BrowseFolder(const char * label,
	std::string & outPath,
	const std::string & defaultPath)
{
	std::future<bool> ret;
	ImGui::BeginGroup();
	ImGui::InputText(label, outPath); ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		ret = std::async(std::launch::async, [&outPath, &defaultPath]() {
			return PickFolder(outPath, defaultPath);
		});
	}
	ImGui::EndGroup();
	return ret;
}

bool GUI::OpenDialog(std::string & outPath,
	const std::vector<std::array<std::string, 2>> & items,
	const std::string & defaultPath)
{
	NFD::UniquePath _outPath;
	std::vector<nfdfilteritem_t> filterItems;
	filterItems.reserve(items.size());
	std::transform(items.begin(), items.end(), std::back_inserter(filterItems),
		[](const auto & a) { return nfdfilteritem_t{a[0].c_str(), a[1].c_str()}; });
	auto result = NFD::OpenDialog(_outPath,
		filterItems.data(), filterItems.size(),
		defaultPath.empty() ? nullptr : defaultPath.c_str());
	if (result == NFD_OKAY)
	{
		outPath = _outPath.get();
		return true;
	}
	else if (result == NFD_CANCEL)
	{
		return false;
	}
	return false;
}

bool GUI::SaveDialog(std::string & outPath,
	const std::vector<std::array<std::string, 2>> & items,
	const std::string & defaultPath,
	const std::string & defaultName)
{
	NFD::UniquePath _outPath;
	std::vector<nfdfilteritem_t> filterItems;
	filterItems.reserve(items.size());
	std::transform(items.begin(), items.end(), std::back_inserter(filterItems),
		[](const auto & a) { return nfdfilteritem_t{a[0].c_str(), a[1].c_str()}; });
	auto result = NFD::SaveDialog(_outPath,
		filterItems.data(), filterItems.size(),
		defaultPath.empty() ? nullptr : defaultPath.c_str(),
		defaultName.empty() ? nullptr : defaultName.c_str());
	if (result == NFD_OKAY)
	{
		outPath = _outPath.get();
		return true;
	}
	else if (result == NFD_CANCEL)
	{
		return false;
	}
	return false;
}

bool GUI::PickFolder(std::string & outPath,
	const std::string & defaultPath)
{
	NFD::UniquePath _outPath;
	auto result = NFD::PickFolder(_outPath,
		defaultPath.empty() ? nullptr : defaultPath.c_str());
	if (result == NFD_OKAY)
	{
		outPath = _outPath.get();
		return true;
	}
	else if (result == NFD_CANCEL)
	{
		return false;
	}
	return false;
}
