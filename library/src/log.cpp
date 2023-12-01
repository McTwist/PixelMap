#include "log.hpp"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#if 0
#include <spdlog/sinks/callback_sink.h>
#endif
#include <spdlog/pattern_formatter.h>
#include <spdlog/stopwatch.h>
#include <spdlog/details/fmt_helper.h>

static auto s_start = spdlog::stopwatch();

class time_since_start : public spdlog::custom_flag_formatter
{
public:
	void format(const spdlog::details::log_msg &, const std::tm &, spdlog::memory_buf_t & dest) override
	{
		auto elapsed = s_start.elapsed();
		auto secs = static_cast<size_t>(elapsed.count());
		auto micr = static_cast<size_t>((elapsed.count() - secs) * 1'000'000);
		if (this->padinfo_.enabled())
		{
			const size_t pad = 0;
			spdlog::details::scoped_padder p(pad, padinfo_, dest);
			spdlog::details::fmt_helper::append_int(secs, dest);
			dest.push_back('.');
			spdlog::details::fmt_helper::pad6(micr, dest);
			return;
		}
		spdlog::details::fmt_helper::append_int(secs, dest);
		dest.push_back('.');
		spdlog::details::fmt_helper::pad6(micr, dest);
	}

	std::unique_ptr<custom_flag_formatter> clone() const override
	{
		return spdlog::details::make_unique<time_since_start>();
	}
};

static std::vector<spdlog::sink_ptr> s_sinks;

void Log::InitFinalize()
{
	auto logger = std::make_shared<spdlog::logger>("pixelmap", s_sinks.begin(), s_sinks.end());
	logger->flush_on(spdlog::level::err);
	spdlog::set_default_logger(std::move(logger));
}

void Log::InitConsole(spdlog::level::level_enum level, bool color)
{
	auto form = std::make_unique<spdlog::pattern_formatter>();
	form->add_flag<time_since_start>('/').set_pattern("[%/] [%^%l%$] %v");
	spdlog::sink_ptr console;
	if (color)
		console = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	else
		console = std::make_shared<spdlog::sinks::stdout_sink_mt>();
	console->set_level(level);
	console->set_formatter(std::move(form));
	s_sinks.emplace_back(std::move(console));
}

void Log::InitFile(spdlog::level::level_enum level, const std::string & file)
{
	auto fp = std::make_shared<spdlog::sinks::basic_file_sink_mt>(file, true);
	fp->set_level(level);
	fp->set_pattern("[%T.%e] [%l] %v");
	s_sinks.emplace_back(std::move(fp));
}

#if 0
void InitCallback(spdlog::level::level_enum level, const std::function<void()> & callback)
{
	auto call = std::make_shared<spdlog::sinks::callback_sink_mt>(callback);
	call->set_level(level);
	call->set_pattern("%v");
	s_sinks.emplace_back(std::move(call));
}
#endif
