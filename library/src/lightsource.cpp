#include "lightsource.hpp"

#include "resource_lightsource_conf.hpp"

#include "string.hpp"

#include <glm/glm.hpp>

#include <fstream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <memory>
#include <sstream>


bool LightSource::read(const std::string & file)
{
	std::function<bool(std::string & line)> getline;
	if (file.empty())
	{
		auto in = std::make_shared<std::istringstream>(std::string(
			resource_lightsource_conf_data,
			resource_lightsource_conf_data + resource_lightsource_conf_size));
		getline = [in](std::string & line) -> bool {
			return bool(std::getline(*in, line));
		};
	}
	else
	{
		auto in = std::make_shared<std::ifstream>(file, std::ios::in | std::ios::binary);
		if (!in->is_open())
			return false;
		getline = [in](std::string & line) -> bool {
			return bool(std::getline(*in, line));
		};
	}
	std::string line;
	while (getline(line))
	{
		// Usage
		// # Comment
		// <namespace_id> = <1-15> [# comment]
		// Example
		// minecraft:lava = 15 # Hot

		// Remove comment
		auto comment = line.find('#');
		if (comment != std::string::npos)
			line.erase(comment);

		string::ltrim(line);
		string::rtrim(line);

		// Optimization to avoid creating too much
		if (line.empty())
			continue;
		
		bool read_value = false;
		std::string ns;
		uint8_t light = 15;
		bool read_space = true;
		int num_spaces = 0;
		// Iterate each character
		for (std::size_t i = 0; i < line.size(); ++i)
		{
			auto c = line[i];
			// Handle whites
			if (string::iswhite(c))
			{
				if (!read_space)
				{
					read_space = true;
					++num_spaces;
				}
				continue;
			}
			else
			{
				read_space = false;
			}

			if (!read_value)
			{
				// Start reading the value to be stored
				if (c == '=')
				{
					read_value = true;
					// Reset space reader
					num_spaces = 0;
					read_space = true;
					continue;
				}

				// Namespace ID
				bool got_namespace = false;
				bool got_invalids = false;
				auto n = i;
				for (; n < line.size(); ++n)
				{
					auto cc = line[n];
					// Namespace delimeter
					if (cc == ':')
					{
						// No namespace
						if (n == i)
							return false;
						// Already has a namespace
						if (got_namespace)
							return false;
						// Invalid namespace
						if (got_invalids)
							return false;
						got_namespace = true;
					}
					else if (string::iswhite(cc))
						break;
					else if (!string::isns(cc))
						got_invalids = true;
				}

				ns = line.substr(i, n - i);
				i += n - i;
				++num_spaces;
			}
			else
			{
				// Verify number
				decltype(i) n;
				for (n = 0; n < 2; ++n)
				{
					auto cc = line[i + n];
					if (string::iswhite(cc))
						break;
					// Not a number
					if (!std::isdigit(cc))
					{
						// Error: Invalid character
						return false;
					}
				}
				light = glm::clamp(std::stoi(line.substr(i, n)), 1, 15);
				i += n-1;
			}
		}

		lights[ns] = light;
	}
	return true;
}

bool LightSource::isLightSource(const std::string & name) const
{
	return lights.find(name) != lights.end();
}

uint8_t LightSource::getLightPower(const std::string & name) const
{
	auto it = lights.find(name);
	return it != lights.end() ? it->second : 0;
}

bool LightSource::hasLightSources() const
{
	return !lights.empty();
}

