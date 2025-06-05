#pragma once
#ifndef WINDOW_HPP
#define WINDOW_HPP

#include <functional>
#include <vector>
#include <tuple>
#include <string>
#include <chrono>
#include <memory>
#include <cstdint>

class Window
{
	struct Framed
	{
		std::function<void()> begin;
		std::function<void()> end;
	};
public:
	// GUI creation
	void create(const std::string & title, int w, int h);
	void destroy();
	bool begin();
	void end();
	void refresh();
	bool alive() const { return run; }
	void close() { run = false; }
	void set_fps(uint32_t fps);

	std::tuple<float, float> get_scale() const;

	/**
	 * @brief Set progress related to the window
	 * @param value Progress of the current task. [0, 1] is normal. [-inf, 0) is indetermined. (1, inf) is done.
	 */
	void progress(float value);

private:
	std::vector<Framed> frames;
	bool run = true, redraw = false;
	std::chrono::duration<float> redraw_time, fps_scale;
	std::chrono::steady_clock::time_point start;
	uint32_t refresh_event = -1;
	std::shared_ptr<struct Data> data;

	void reset_redraw();
};

#endif // WINDOW_HPP
