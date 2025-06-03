#pragma once
#ifndef STRING_HPP
#define STRING_HPP

#include <string>
#include <sstream>
#include <algorithm>
#include <cstdint>

/**
 * @brief String namespace
 * Strings handling made easier.
 */
class string
{
	/**
	 * Private implementation
	 */
	template<typename T>
	static inline void _format(std::ostream & o, T t)
	{
		o << t;
	}

	/**
	 * Private implementation
	 * Recursive variadic
	 */
	template<typename T, typename... Args>
	static inline void _format(std::ostream & o, T t, Args... args)
	{
		_format(o, t);
		_format(o, args...);
	}

public:
	// Avoid creation
	string() = delete;
	/**
	 * @brief Formats a list of variables into one string
	 * @tparam Args Unknown type of parameter to be converted
	 * @param sections Unknown variadic variables to be converted
	 * @return A combined string from sorted input
	 */
	template<typename... Args>
	static inline std::string format(Args... sections)
	{
		std::ostringstream o;
		_format(o, sections...);
		return o.str();
	}

	// https://stackoverflow.com/a/217605
	static inline std::string & ltrim(std::string & s)
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](uint8_t c) {
			return !std::isspace(c);
		}));
		return s;
	}

	// https://stackoverflow.com/a/217605
	static inline std::string & rtrim(std::string & s)
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), [](uint8_t c) {
			return !std::isspace(c);
		}).base(), s.end());
		return s;
	}

	// Check if a character is whitespace or null
	static inline bool iswhite(int c)
	{
		return std::isspace(c) || c == '\0';
	}

	// Check if character is a valid namespace name
	// Note: Uses BE as it makes JE backwards compatible
	static inline bool isns(int c)
	{
		// Java Edition
		//return (std::isalpha(c) && std::islower(c)) || std::isdigit(c);
		// Bedrock Edition
		return c != '/' && c != ':';
	}
};

#endif // STRING_HPP
