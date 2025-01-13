#pragma once
#ifndef MODULE_HPP
#define MODULE_HPP

#include "library.hpp"

#include <memory>
#include <string>

class Module : public Library
{
public:
	bool load(const std::string & path)
	{
		if (!Library::load(path))
			return false;
		auto vf = getFunction<int()>("module_version");
		if (vf)
			_version = vf();
		else
			_version = -1;
		return true;
	}

	inline int version() const
	{
		return _version;
	}

	template<class T>
	std::shared_ptr<T> createIntance(const std::string & instanceName)
	{
		using pT = T *;
		const std::string constructName = "module_" + instanceName + "_construct";
		const std::string destructName = "module_" + instanceName + "_destruct";
		auto construct = getFunction<pT()>(constructName);
		auto destruct = getFunction<void(pT)>(destructName);
		if (!construct || !destruct)
			return nullptr;
		return std::shared_ptr<T>(construct(), [destruct](pT p) { destruct(p); });
	}
private:
	int _version;
};

#endif // MODULE_HPP
