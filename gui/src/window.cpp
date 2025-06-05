#include "window.hpp"

#include "icon.hpp"
#include "resource_DroidSans_ttf.hpp"

#include "imgui/imgui_custom.h"

#include <imgui.h>
#include <backends/imgui_impl_sdl2.h>
#ifdef USE_OPENGL
#include <backends/imgui_impl_opengl3.h>
#else
#include <backends/imgui_impl_sdlrenderer2.h>
#endif
#include <spdlog/spdlog.h>
#include <nfd.hpp>

#include <SDL.h>
#include <SDL_opengl.h>

#include <algorithm>

using namespace std::literals::chrono_literals;

struct Data
{
	SDL_Window * window;
#ifdef USE_OPENGL
	SDL_GLContext gl_context;
#else
	SDL_Renderer * renderer;
#endif
	ImVec4 clear_color = {0.45f, 0.55f, 0.60f, 1.0f};
};

void Window::create(const std::string & title, int w, int h)
{
	data = std::make_shared<Data>();
	if (SDL_Init(SDL_INIT_VIDEO))
	{
		spdlog::error("SDL: {:s}", SDL_GetError());
		return;
	}

	refresh_event = SDL_RegisterEvents(1);
	if (refresh_event == (uint32_t)-1)
	{
		spdlog::error("SDL: No events left");
	}

#ifdef USE_OPENGL
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
#endif
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
	data->window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, window_flags);
	if (data->window == nullptr)
	{
		spdlog::error("SDL: Unable to create window");
		return;
	}
#ifdef USE_OPENGL
	data->gl_context = SDL_GL_CreateContext(data->window);
	if (data->gl_context == nullptr)
	{
		spdlog::error("SDL: Unable to create GL context");
		return;
	}
	SDL_GL_MakeCurrent(data->window, data->gl_context);
	SDL_GL_SetSwapInterval(1); // Enable vsync
#else
	data->renderer = SDL_CreateRenderer(data->window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
	if (data->renderer == nullptr)
	{
		spdlog::error("SDL: Unable to get renderer");
		return;
	}
#endif

	// Load icon
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
	
#ifdef USE_OPENGL
	if (!ImGui_ImplSDL2_InitForOpenGL(data->window, data->gl_context))
	{
		spdlog::error("SDL: Unable to init OpenGL");
		return;
	}
	if (!ImGui_ImplOpenGL3_Init(glsl_version))
	{
		spdlog::error("SDL: Unable to init OpenGL3");
		return;
	}
#else
	auto [scale_width, scale_height] = get_scale();
	SDL_RenderSetScale(data->renderer, scale_width, scale_height);
	io.FontGlobalScale = 1.f / scale_width;
	if (!ImGui_ImplSDL2_InitForSDLRenderer(data->window, data->renderer))
	{
		spdlog::error("SDL: Unable to init SDL renderer");
		return;
	}
	if (!ImGui_ImplSDLRenderer2_Init(data->renderer))
	{
		spdlog::error("SDL: Unable to init SDL2 renderer");
		return;
	}
#endif

	// Draw order
#ifdef USE_OPENGL
	frames.emplace_back(Framed{ ImGui_ImplOpenGL3_NewFrame, [this, io](){
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		glClearColor(data->clear_color.x * data->clear_color.w,
			data->clear_color.y * data->clear_color.w,
			data->clear_color.z * data->clear_color.w,
			data->clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	} });
	frames.emplace_back(Framed{ ImGui_ImplSDL2_NewFrame, [this](){ SDL_GL_SwapWindow(data->window); } });
#else
	frames.emplace_back(Framed{ ImGui_ImplSDLRenderer2_NewFrame, [this](){ SDL_RenderPresent(data->renderer); } });
	frames.emplace_back(Framed{ ImGui_ImplSDL2_NewFrame, [this](){
		SDL_SetRenderDrawColor(data->renderer,
			data->clear_color.x * data->clear_color.w * 255,
			data->clear_color.y * data->clear_color.w * 255,
			data->clear_color.z * data->clear_color.w * 255,
			data->clear_color.w * 255);
		SDL_RenderClear(data->renderer);
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), data->renderer);
	} });
#endif
	frames.emplace_back(Framed{ ImGui::NewFrame, ImGui::Render });

	NFD::Init();

	start = std::chrono::steady_clock::now();
	set_fps(60);
	reset_redraw();
	// Refresh for the first few frames
	refresh();
}

void Window::destroy()
{
	NFD::Quit();

#ifdef USE_OPENGL
	ImGui_ImplOpenGL3_Shutdown();
#else
	ImGui_ImplSDLRenderer2_Shutdown();
#endif
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

#ifdef USE_OPENGL
	SDL_GL_DeleteContext(data->gl_context);
#else
	SDL_DestroyRenderer(data->renderer);
#endif
	SDL_DestroyWindow(data->window);
	SDL_Quit();
	data.reset();
}

bool Window::begin()
{
	SDL_Event event;
	redraw = redraw_time > 0s;
	auto got_event = redraw ? SDL_PollEvent(&event) : SDL_WaitEvent(&event);
	if (got_event)
	{
		reset_redraw();
		do
		{
			ImGui_ImplSDL2_ProcessEvent(&event);
			if (event.type == SDL_QUIT)
				run = false;
			if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(data->window))
				run = false;
		}
		while(SDL_PollEvent(&event));
	}
	auto next = std::chrono::steady_clock::now();
	if (redraw_time > 0s)
	{
		redraw_time -= next - start;
		start = next;
		redraw = true;
	}
	else
	{
		start = next;
		redraw = false;
	}
	if (redraw)
		std::for_each(frames.begin(), frames.end(), [](auto & frame) { frame.begin(); });
	return redraw;
}

void Window::end()
{
	if (redraw)
		std::for_each(frames.rbegin(), frames.rend(), [](auto & frame) { frame.end(); });
	auto durr = std::chrono::steady_clock::now() - start;
	if (durr < fps_scale)
		std::this_thread::sleep_for(fps_scale - durr); // "vsync"
}

void Window::refresh()
{
	SDL_Event event;
	event.type = refresh_event;
	SDL_PushEvent(&event);
}

void Window::set_fps(uint32_t fps)
{
	fps_scale = 1.s / fps;
}

std::tuple<float, float> Window::get_scale() const
{
	int window_width = 0, window_height = 0,
		render_output_width = 0, render_output_height = 0;
	SDL_GetWindowSize(data->window, &window_width, &window_height);
	SDL_GetRendererOutputSize(data->renderer, &render_output_width, &render_output_height);
	return std::make_tuple(
		float(render_output_width) / float(window_width),
		float(render_output_height) / float(window_height)
	);
}

void Window::progress(float value)
{
#if SDL_VERSION_ATLEAST(3, 4, 0)
	auto old_state = SDL_GetWindowProgressSate(data->window);
	SDL_ProgressState new_state;
	if (value < 0)
		new_state = SDL_PROGRESS_STATE_INDETERMINATE;
	else if (value > 1)
		new_state = SDL_PROGRESS_STATE_NONE;
	else
		new_state = SDL_PROGRESS_STATE_NORMAL;
	if (new_state != old_state)
		SDL_SetWindowProgressState(data->window, new_state);
	if (0 <= value && value <= 1)
		SDL_SetWindowProgressValue(data->window, value);
#else
	(void)value;
#endif
}

void Window::reset_redraw()
{
	redraw_time = fps_scale * 16;
}
