#pragma once
#ifndef PLATFORM_HPP
#define PLATFORM_HPP

#include <vector>
#include <string>
#include <memory>

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
	#include <unistd.h>
	#define PLATFORM_UNIX
	#if defined(_POSIX_VERSION)
		#define PLATFORM_POSIX
	#endif
#endif

#if defined(_WIN32)
	#define PLATFORM_WINDOWS
#elif defined(__APPLE__)
	#define PLATFORM_APPLE
#elif defined(__linux__)
	#define PLATFORM_LINUX
#else
	#error "Unknown compiler"
#endif

// Platform namespace
/**
 * @brief Platform namespace
 * Handles certain functionalities that is different between platforms.
 */
namespace platform
{
	const char pathSepWin = '\\';
	const char pathSepUnix = '/';

	/**
	 * Path separator. May or may not work perfectly, as some platforms
	 * can deal with both.
	 */
#ifdef PLATFORM_WINDOWS
	const char pathSep = pathSepWin;
#else
	const char pathSep = pathSepUnix;
#endif

	/**
	 * @brief Path namespace
	 * If it got a path, this got it covered.
	 */
	namespace path
	{
		/**
		 * @brief Join a set of strings into one unified path
		 * @param sections A list of strings
		 * @return A path built from a list of strings
		 */
		std::string join(const std::vector<std::string> & sections);

		/**
		 * @brief Join a set of strings into one unified path
		 * @tparam T Type should be some sort of string
		 * @param sections A list of sections that can be passed to join
		 * @return A path build from a list of strings
		 */
		template<typename ...T>
		inline std::string join(T... sections)
		{
			return join(std::vector<std::string>{sections...});
		}

		/**
		 * @brief Unify a path delimeters
		 * @param path The path to unify
		 * @return A unified path
		 * Unifies a path by replacing delimeters with platform specifics.
		 */
		std::string unify(const std::string & path);


		/**
		 * @brief Get environment variable
		 * @param var The variable name
		 * @return The variable value if it exists
		 */
		std::string getenv(const std::string & var);

		/**
		 * @brief Make a directory, recursively
		 * @param path A path to create
		 * @return True if exist or created, false if error
		 */
		bool mkdir(const std::string & path);
	}

	/**
	 * @brief File descriptor namespace
	 * Handles file descriptors.
	 */
	namespace fd
	{
		/**
		 * @brief Current max amount of file descriptors
		 * @return The current max of file dscriptors
		 */
		std::size_t max();
	}

	/**
	 * @brief Memory map namespace
	 * Handles file mapped memory.
	 */
	namespace mmap
	{
		/**
		 * @brief Load the file to memory
		 * @param file The file to load
		 * @return Valid pointer for success, NULL if error
		 */
		std::shared_ptr<void> load(const std::string & file);
	}

} // platform

#endif // PLATFORM_HPP
