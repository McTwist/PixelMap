#include "module/library.hpp"
#include "platform.hpp"

/*
 * This library is meant to dynamically load modules/libraries
 * to enable certain features that makes it either execute the
 * pipeline faster, or adds more features. Currently it is
 * unknown what is best to add, but some are suggested below:
 * - OpenGL (render)
 * - OpenCL (parse/render)
 * - Vulcan (parse/render)
 * - Add more render modes (Need to add specific support for that)
 */

// Windows
#if defined(PLATFORM_WINDOWS)
#include <windows.h>

struct lib_impl
{
	HINSTANCE lib = nullptr;
};

#define libload(path) LoadLibraryEx((path), NULL, DONT_RESOLVE_DLL_REFERENCES)
#define libunload(lib) FreeLibrary((lib))
#define libsymbol(lib, name) GetProcAddress((lib), (name))
#define liberror() (GetLastError() != 0)
#define liberrormsg() GetLastErrorMessage()
#define liberrorreset() GetLastError()

// Internal function to get latest error message
[[maybe_unused]]
inline static std::string GetLastErrorMessage()
{
	auto msgID = GetLastError();
	if (msgID == 0)
		return std::string();
	LPSTR msgBuf = nullptr;
	auto size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, msgID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&msgBuf, 0, NULL
	);

	std::string msg(msgBuf, size);

	LocalFree(msgBuf);

	return msg;
}

// Linux, Mac
#elif defined(PLATFORM_UNIX)
#include <dlfcn.h> // -ldl

struct lib_impl
{
	void * lib = nullptr;
};

static std::string g_previus_error;

#define libload(path) dlopen((path), RTLD_LAZY)
#define libunload(lib) dlclose((lib))
#define libsymbol(lib, name) dlsym((lib), (name))

[[maybe_unused]]
inline static bool liberror()
{
	auto err = dlerror();
	if (err)
		g_previus_error = err;
	return !g_previus_error.empty();
}
[[maybe_unused]]
inline static std::string liberrormsg()
{
	auto err = dlerror();
	if (err)
		g_previus_error = err;
	return g_previus_error;
}
[[maybe_unused]]
inline static void liberrorreset()
{
	dlerror();
	g_previus_error.clear();
}

// Unknown
#else
//#error "No support for loading libraries"
// Minimal implementation to ensure compilation, but error when using it
struct lib_impl { void * lib; };
#define libload(path) ((void*)0)
#define libunload(lib) ((void*)0)
#define libsymbol(lib, name) ((void*)0)
#define liberror() (true)
#define liberrormsg() (std::string("No support for dynamic library loading"))
#define liberrorreset() ((void *)0)
#endif

Library::Library()
{
	impl = std::make_shared<lib_impl>();
}

Library::~Library()
{
	unload();
}

bool Library::load(const std::string & path)
{
	liberrorreset();
	impl->lib = libload(path.c_str());
	if (!impl->lib)
		error = liberrormsg();
	return !!impl->lib;
}

void Library::unload()
{
	liberrorreset();
	if (impl->lib && !libunload(impl->lib))
		error = liberrormsg();
	impl->lib = decltype(impl->lib)();
}

bool Library::isLoaded() const
{
	return !!impl->lib;
}

const std::string & Library::getError() const
{
	return error;
}

bool Library::hasError() const
{
	return !error.empty();
}

// Get a function symbol from the library
func_type Library::getFuncSymbol(const std::string & name)
{
	return reinterpret_cast<func_type>(getSymbol(name));
}

// Get a symbol from the library
void_type Library::getSymbol(const std::string & name)
{
	liberrorreset();
	if (!impl->lib)
		return nullptr;
	// Cast directly so it can be handled later on
	auto var = reinterpret_cast<void_type>(libsymbol(impl->lib, name.c_str()));
	if (!var)
		error = liberrormsg();
	return var;
}
