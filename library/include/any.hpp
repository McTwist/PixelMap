#pragma once
#ifndef ANY_HPP

#include <string>
#include <memory>
#include <cstring>
#include <typeinfo>
#include <typeindex>

// TODO: Replace with std::any (C++17)
class Any;

namespace cast
{
	template<typename T>
	inline T fromString(const std::string & str)
	{
		return str;
	}
	template<>
	inline float fromString<float>(const std::string & str)
	{
		return std::stof(str);
	}
	template<>
	inline double fromString<double>(const std::string & str)
	{
		return std::stod(str);
	}
	template<>
	inline long double fromString<long double>(const std::string & str)
	{
		return std::stold(str);
	}
	template<>
	inline int fromString<int>(const std::string & str)
	{
		return std::stoi(str);
	}
	template<>
	inline long fromString<long>(const std::string & str)
	{
		return std::stol(str);
	}
	template<>
	inline long long fromString<long long>(const std::string & str)
	{
		return std::stoll(str);
	}
	template<>
	inline unsigned long fromString<unsigned long>(const std::string & str)
	{
		return std::stoul(str);
	}
	template<>
	inline unsigned long long fromString<unsigned long long>(const std::string & str)
	{
		return std::stoull(str);
	}
}

/**
 * Stores any variable that is thrown at it
 * Will never convert any type
 */
class Any
{
public:
	Any() = default;
	template<typename T>
	Any(const T & var)
	{
		set<T>(var);
	}

	// Specifically set type with var
	template<typename T>
	Any & operator=(const T & var)
	{
		set<T>(var);
		return *this;
	}

	// Set var into the type
	template<typename T>
	void set(const T & var)
	{
		_data = std::make_shared<T>(var);
		_size = sizeof(T);
		_type = typeid(T);
	}

	// Get var from type
	template<typename T>
	const T & get() const
	{
		if (!isType<T>())
			throw std::bad_cast();
		return *reinterpret_cast<T *>(_data.get());
	}

	// Size of the type
	inline std::size_t size() const { return _size; }

	// Compare the type with an another type
	inline bool operator==(const Any & b) const
	{
		return _type == b._type &&
			std::memcmp(_data.get(), b._data.get(), _size) == 0;
	}
	inline bool operator!=(const Any & b) const
	{
		return !operator==(b);
	}

	// Check if the type is correct type
	template<typename T>
	bool isType() const
	{
		return std::type_index(typeid(T)) == _type;
	}

	// Get type index for current type
	std::type_index getType() const { return _type; }

	// Validate the type
	operator bool() const noexcept { return _data.operator bool(); }

	// Convert from string to any type
	template<typename T>
	static Any fromString(const std::string & str)
	{
		try
		{
			return Any(cast::fromString<T>(str));
		}
		catch (...)
		{
			return Any();
		}
	}

private:
	std::shared_ptr<void> _data;
	std::size_t _size = 0;
	std::type_index _type = typeid(void);
};

#endif // ANY_HPP
