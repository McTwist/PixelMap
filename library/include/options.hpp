#pragma once
#ifndef OPTIONS_HPP
#define OPTIONS_HPP

#include "any.hpp"

#include <string>
#include <unordered_map>

/**
 * @brief Keeps track of options to be used
 */
class Options
{
public:
	/**
	 * @brief Checks if the option exist
	 * @param opt The name of the option
	 * @return True if exist, false otherwise
	 */
	inline bool has(const std::string & opt) const noexcept
	{
		return options.find(opt) != options.end();
	}

	/**
	 * @brief Set the option
	 * @param opt The name of the option
	 * @param var The variable to set
	 */
	inline void set(const std::string & opt, Any var) noexcept
	{
		auto it = options.find(opt);
		if (it != options.end())
			it->second = var;
		else
			options.insert(std::make_pair(opt, var));
	}

	/**
	 * @brief Get the option value or default
	 * @param opt The name of the option
	 * @param def Default value if not exist
	 * @return The value of option, or default if not exist
	 */
	inline Any get(const std::string & opt, Any def = Any()) const noexcept
	{
		auto it = options.find(opt);
		if (it == options.end()) return def;
		return it->second;
	}

	/**
	 * @brief Get the option value or default
	 * @tparam The type of the option value
	 * @param opt The name of the option
	 * @param def Default value if not exist
	 * @return The value of option, or default if not exist
	 * May throw exception if type is invalid.
	 */
	template<typename T>
	T get(const std::string & opt, T def = T()) const
	{
		auto it = options.find(opt);
		if (it == options.end()) return def;
		return it->second.get<T>();
	}

	/**
	 * @brief Array operator to get value from option
	 * @param opt The name of the option
	 * @return The option value
	 * May throw exception if option does not exist.
	 */
	inline Any & operator[](const std::string & opt)
	{
		return options.at(opt);
	}

	/**
	 * @brief Array operator to get value from option
	 * @param opt The name of the option
	 * @return The option value
	 * May throw exception if option does not exist.
	 */
	inline const Any & operator[](const std::string & opt) const
	{
		return options.at(opt);
	}

	/**
	 * @brief Merge two options, overwriting existing options
	 * @param opt The options to merge
	 */
	inline void merge(const Options & opt)
	{
		auto opts = opt.options;
		opts.insert(options.begin(), options.end());
		std::swap(options, opts);
	}

	/**
	 * @brief Clear options
	 */
	inline void clear()
	{
		options.clear();
	}

private:
	// The options values
	std::unordered_map<std::string, Any> options;
};

#endif // OPTIONS_HPP
