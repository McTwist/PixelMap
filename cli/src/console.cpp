#include "console.hpp"

#include "platform.hpp"

#include <spdmon/spdmon.hpp>

struct Data
{
	std::shared_ptr<spdmon::LoggerProgress> monitor;
};

Console::Console()
{
	std::ios::sync_with_stdio(false);
	data = std::make_shared<Data>();
	auto level = spdlog::get_level();
	if (level <= spdlog::level::info)
	{
		data->monitor = std::make_shared<spdmon::LoggerProgress>(spdlog::default_logger(), "", false, 0, level);
		spdlog::set_level(level); // spdmon bugs this out
		data->monitor->SetLbarFmt("{desc}: {frac:3.0f}% |");
		data->monitor->SetRbarFmt("| {n}/{total} [{elapsed:.2%T} / {remaining:.2%T}]{eol}");
		data->monitor->SetNoTotalFmt("{desc}: {n} [{elapsed:.2%T}]{eol}");
	}
}

void Console::progress(uint32_t count, uint32_t current, const std::string & status)
{
	if (spdlog::get_level() <= spdlog::level::info)
	{
		current = (std::min)(current, count);
		if (current > data->monitor->Count())
		{
			data->monitor->SetTotal(count);
			data->monitor->Update(current - data->monitor->Count());
		}
		else if (current < data->monitor->Count())
		{
			data->monitor->Restart(status, count);
		}
	}
}

