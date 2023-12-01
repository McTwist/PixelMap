#include "programoptions.hpp"

#include <queue>
#include <iostream>

ProgramOptions::ProgramOptions(int _argc, const char ** _argv, const std::string & _description) :
	argc(_argc), argv(_argv), description(_description)
{
}

// Parameter with short name
void ProgramOptions::addParam(const std::string & name,
	char s, uint32_t args, Convert convert)
{
	parameters_short.emplace(std::make_pair(s, ParameterData{name, args, convert}));
}
// Parameter with long name
void ProgramOptions::addParam(const std::string & name,
	const std::string & l, uint32_t args, Convert convert)
{
	parameters_long.emplace(std::make_pair(l, ParameterData{name, args, convert}));
}
// Parameter with both short and long name
void ProgramOptions::addParam(const std::string & name,
	char s, const std::string & l, uint32_t args, Convert convert)
{
	addParam(name, s, args, convert);
	addParam(name, l, args, convert);
}

void ProgramOptions::addHelp(const std::string & name, const std::string & desc)
{
	help.emplace(std::make_pair(name, desc));
}

bool ProgramOptions::parse()
{
	errors.clear();
	// Store argument parameter
	program.assign(argv[0]);
	if (argc == 1)
		return true;
	// Note: C99 standard requires the arguments to be null-terminated
	std::vector<std::string> args(argv + 1, argv + argc);

	bool ret = true;
	bool no_more = false;
	std::queue<ParameterData> argument_var;

	for (auto arg : args)
	{
		// Should never happen
		if (arg.empty())
			continue;
		// Engulf everything
		if (!argument_var.empty())
		{
			auto param = argument_var.front();
			auto var = param.convert ? param.convert(arg) : arg;
			if (var)
				arguments[param.name].emplace_back(var);
			// ERROR
			else
			{
				error("Invalid type");
				ret = false;
			}
			argument_var.pop();
			continue;
		}
		// Normal arguments, store them
		if (no_more || arg[0] != '-')
		{
			normal.emplace_back(arg);
			continue;
		}
		if (arg.size() == 1)
		{
			// ERROR
			error("Empty parameter");
			ret = false;
			continue;
		}
		// Short arguments
		if (arg[1] != '-')
		{
			for (decltype(arg.size()) n = 1; n < arg.size(); ++n)
			{
				auto it = parameters_short.find(arg[n]);
				if (it == parameters_short.end())
				{
					// ERROR
					error("Parameter " + std::string(1, arg[n]) + " does not exist");
					ret = false;
					continue;
				}
				arguments.insert(std::make_pair(it->second.name, std::vector<Any>()));
				// Additional argument
				for (decltype(it->second.arguments) m = 0; m < it->second.arguments; ++m)
					argument_var.push(it->second);
			}
			continue;
		}
		// No more arguments
		if (arg.size() == 2)
		{
			no_more = true;
			continue;
		}
		// Long arguments
		auto it = parameters_long.find(arg.substr(2));
		if (it == parameters_long.end())
		{
			// ERROR
			error("Parameter " + arg + " does not exist");
			ret = false;
			continue;
		}
		arguments.insert(std::make_pair(it->second.name, std::vector<Any>()));
		// Additional argument
		for (decltype(it->second.arguments) m = 0; m < it->second.arguments; ++m)
			argument_var.push(it->second);
	}

	if (!argument_var.empty())
	{
		// ERROR
		error("Missing argument for parameter");
		ret = false;
	}

	return ret;
}

void ProgramOptions::printHelp() const
{
	const auto getShort = [this](const std::string & name) -> char
	{
		for (auto parameter : parameters_short)
			if (parameter.second.name == name)
				return parameter.first;
		return '\0';
	};
	const auto getLong = [this](const std::string & name) -> std::string
	{
		for (auto parameter : parameters_long)
			if (parameter.second.name == name)
				return parameter.first;
		return std::string();
	};
	std::cout << description << std::endl;
	std::cout << "Options:" << std::endl;
	for (const auto h : help)
	{
		auto s = getShort(h.first);
		auto l = getLong(h.first);
		std::cout << "  ";
		if (s != 0)
			std::cout << "-" << s;
		else
			std::cout << "  ";
		std::cout << " ";
		if (!l.empty())
			std::cout << "--" << l;
		else
			std::cout << "  ";
		std::cout << std::string(15 + 2 - l.size(), ' ');
		std::cout << h.second << std::endl;
	}
}

void ProgramOptions::error(const std::string & err)
{
	errors.emplace_back(err);
}
