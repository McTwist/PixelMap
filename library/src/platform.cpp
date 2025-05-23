#include "platform.hpp"

#include "semaphore.hpp"

#include <algorithm>
#include <filesystem>

#ifdef PLATFORM_WINDOWS
#include <direct.h>
#include <windows.h> // mmap
#include <memoryapi.h> // mmap
#ifdef max
#undef max
#endif
#else
#include <sys/resource.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace platform
{
namespace path
{

// Join a list of strings with pathSep as delimiter
std::string join(const std::vector<std::string> & sections)
{
	std::string path;
	std::string sep(1, pathSep);
	std::string emp;
	for (const auto & section : sections)
		path = path + (path.empty() ? emp : sep) + section;
	return path;
}

// Unify a path delimeters
std::string unify(const std::string & path)
{
#ifdef PLATFORM_WINDOWS
	auto rep = pathSepUnix;
#else
	auto rep = pathSepWin;
#endif
	std::string uni = path;
	std::replace(uni.begin(), uni.end(), rep, pathSep);
	return uni;
}

// Get environment variable
std::string getenv(const std::string & var)
{
#ifdef PLATFORM_WINDOWS
	char * value = nullptr;
#if _MSC_VER
	size_t len = 0;
	if (_dupenv_s(&value, &len,	var.c_str()) || !value)
		return std::string();
	std::string _env(value, strnlen(value, len));
	free(value);
#else
	if (!(value = std::getenv(var.c_str())))
		return std::string();
	std::string _env(value, strlen(value));
#endif
	return _env;
#else
	return std::getenv(var.c_str());
#endif
}

// Create path if it does not exist
// Returns false if it failed
bool mkdir(const std::string & path)
{
	return std::filesystem::create_directories(path);
}

} // path

namespace fd
{

static semaphore file_counter(max());
static std::size_t current_fd_count = 512UL;

std::size_t max()
{
#if defined(PLATFORM_WINDOWS)
	/* While Win10 standard is 8192 for the hard limit for a process,
	 * there is technically a soft limit of 512, which could be increased.
	 * https://stackoverflow.com/a/870224
	 */
	return current_fd_count;
#elif defined(PLATFORM_UNIX)
	/* The soft limit should normally be used, but the limit could be
	 * increased if necessary.
	 */
	struct rlimit limit;
	if (getrlimit(RLIMIT_NOFILE, &limit) != 0)
		// Default value
		return current_fd_count;
	return limit.rlim_cur;
#else
	// Best number we can do
	return 128;
#endif
}

} // fd

namespace mmap
{

std::shared_ptr<void> load(const std::string & file)
{
#if defined(PLATFORM_WINDOWS)
	auto handle = OpenFileMappingA(FILE_MAP_READ, FALSE, file.c_str());
	if (!handle)
		return std::shared_ptr<void>();
	auto ptr = MapViewOfFile(handle, FILE_MAP_READ, 0, 0, 0);
	if (!handle)
	{
		CloseHandle(handle);
		return std::shared_ptr<void>();
	}
	return std::shared_ptr<void>(ptr, [handle](void * ptr)
	{
		UnmapViewOfFile(ptr);
		CloseHandle(handle);
	});
#elif defined(PLATFORM_UNIX)
	auto fd = open(file.c_str(), O_RDONLY);
	if (!fd)
		return std::shared_ptr<void>();
	auto ptr = ::mmap(NULL, 0, PROT_READ, MAP_PRIVATE, fd, 0);
	if (!ptr)
	{
		close(fd);
		return std::shared_ptr<void>();
	}
	return std::shared_ptr<void>(ptr, [fd](void * ptr)
	{
		munmap(ptr, 0);
		close(fd);
	});
#else
	return std::shared_ptr<void>();
#endif
}

} // mmap

} // platform
