#pragma once
#ifndef LOG_HPP
#define LOG_HPP

#include <spdlog/spdlog.h>

#include <string>
#include <functional>

namespace Log
{
	void InitConsole(spdlog::level::level_enum level, bool color);
	void InitFile(spdlog::level::level_enum level, const std::string & file);
#if 0
	void InitCallback(spdlog::level::level_enum level, const std::function<void()> & callback);
#endif
	void InitFinalize();
}

#endif // LOG_HPP
