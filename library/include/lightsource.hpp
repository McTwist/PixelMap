#pragma once
#ifndef LIGHTSOURCE_HPP
#define LIGHTSOURCE_HPP

#include <string>
#include <unordered_map>
#include <cstdint>

class LightSource
{
public:
	bool read(const std::string & file = {});
	bool isLightSource(const std::string & name) const;
	uint8_t getLightPower(const std::string & name) const;
	bool hasLightSources() const;
private:
	std::unordered_map<std::string, uint8_t> lights;
};

#endif // LIGHTSOURCE_HPP
