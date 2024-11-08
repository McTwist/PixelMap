#pragma once
#ifndef PROGRAMOPTIONS_HPP
#define PROGRAMOPTIONS_HPP

#include "any.hpp"

#include <string>
#include <map>
#include <vector>
#include <functional>

/**
 * Parses the arguments for the program
 * Not guarenteed to be secure, but does its job
 */
class ProgramOptions
{
	/**
	* A function to convert a string to Any value.
	*/
	typedef std::function<Any(const std::string &)> Convert;
public:
	// Note: Needed?
	enum class Error
	{
		PARAM_EMPTY,
		PARAM_NONEXISTANT,
		PARAM_MISSING_ARG,
	};

	ProgramOptions(int argc, const char ** argv, const std::string & description = std::string());

	// Adding a parameter
	// Can handle amount of arguments and automatic conversion and validation
	void addParam(const std::string & name,
		char s, uint32_t args = 0, Convert convert = nullptr);
	void addParam(const std::string & name,
		const std::string & l, uint32_t args = 0, Convert convert = nullptr);
	void addParam(const std::string & name, char s,
		const std::string & l, uint32_t args = 0, Convert convert = nullptr);

	// Adding a parameter with a certain type
	template<typename T>
	void addParamType(const std::string & name,
		char s, uint32_t args = 0)
	{
		addParam(name, s, args, Any::fromString<T>);
	}
	template<typename T>
	void addParamType(const std::string & name,
		const std::string & l, uint32_t args = 0)
	{
		addParam(name, l, args, Any::fromString<T>);
	}
	template<typename T>
	void addParamType(const std::string & name,
		char s, const std::string & l, uint32_t args = 0)
	{
		addParam(name, s, l, args, Any::fromString<T>);
	}

	// Add help description
	void addHelp(const std::string & name, const std::string & desc);

	// Parse the arguments
	// Returns true on success, else check getErrors for reason
	bool parse();

	// Displays help text
	void printHelp() const;

	// Get current errors
	const std::vector<std::string> & getErrors() const { return errors; }

	// Get parameters
	const std::map<std::string, std::vector<Any>> & getParameters() const { return arguments; }
	// Get arguments
	const std::vector<std::string> & getArguments() const { return normal; }

private:
	int argc;
	const char ** argv;

	struct ParameterData
	{
		std::string name;
		uint32_t arguments;
		Convert convert;
	};

	std::map<char, ParameterData> parameters_short;
	std::map<std::string, ParameterData> parameters_long;

	std::string description;
	std::map<std::string, std::string> help;

	std::map<std::string, std::vector<Any>> arguments;
	std::vector<std::string> normal;
	std::string program;

	void error(const std::string & err);
	std::vector<std::string> errors;
};

#endif // PROGRAMOPTIONS_HPP
