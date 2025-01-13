#pragma once
#ifndef LIBRARY_HPP
#define LIBRARY_HPP

#include <string>
#include <functional>
#include <memory>

// Default function type to ensure compatibility over compilers
typedef void (*func_type)();
typedef void * void_type;

/**
 * Used to load a library and its symbols
 */
class Library
{
public:
	Library();
	~Library();
	// Load a library from path
	bool load(const std::string & path);
	// Unload this library
	void unload();
	// Check if the library is loaded
	bool isLoaded() const;
	// Get latest error
	const std::string & getError() const;
	// Check if there has been any errors
	bool hasError() const;

	// Get a function, converted to proper type
	template<typename T>
	std::function<T> getFunction(const std::string & name)
	{
		return std::function<T>(reinterpret_cast<T *>(getFuncSymbol(name)));
	}

	// Get a variable, converted to correct type
	template<typename T>
	T * getVariable(const std::string & name)
	{
		return reinterpret_cast<T *>(getSymbol(name));
	}

private:
	std::shared_ptr<struct lib_impl> impl;
	std::string error;

	// Get a symbol from the library
	func_type getFuncSymbol(const std::string & name);
	void_type getSymbol(const std::string & name);
};

#endif // LIBRARY_HPP
