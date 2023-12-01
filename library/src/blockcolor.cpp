#include "blockcolor.hpp"

#include "resource_blockcolor_conf.hpp"

#include "string.hpp"

#include <fstream>
#include <algorithm>
#include <cctype>
#include <vector>
#include <memory>
#include <sstream>


// Note: This function is a mess, but it works.
// Wonder if it should be changed.
bool BlockColor::read(const std::string & file)
{
	std::function<bool(std::string & line)> getline;
	if (file.empty())
	{
		auto in = std::make_shared<std::istringstream>(std::string(
			resource_blockcolor_conf_data,
			resource_blockcolor_conf_data + resource_blockcolor_conf_size));
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
		// <block_id|namespace_id>[:<damage_value>[...]][ ...] = <hex_color>|<r g b[ a]> [# comment]
		// Example
		// 1 2:3:4:6 5:23 67 minecraft:stone = 456789 # Assign everything

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
		std::vector<uint16_t> blockids;
		std::vector<std::string> nsids;
		utility::RGBA color;
		color.a = 255;
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

				// Block ID
				// Note: According to standard, a namespace could start with a number
				// Maybe handle this in another manner later on
				if (std::isdigit(c))
				{
					// Current status reading the blocks
					struct block_status
					{
						int n, m;
						uint16_t block;
						bool has_damage = false;
					} status;
					// Easy to use function to read a block with damage
					auto read_block = [&status, i, &blockids, &line]()
					{
						if (status.n == status.m)
						{
							// Error: No number found
							return false;
						}

						auto v = std::stoul(line.substr(i + std::size_t(status.m), std::size_t(status.n - status.m)));

						if (v > std::numeric_limits<uint16_t>::max())
						{
							// Error: Too big number
							return false;
						}

						if (status.m == 0)
							status.block = static_cast<uint16_t>(v);
						else
						{
							blockids.push_back(status.block | static_cast<uint16_t>(v << 12));
							status.has_damage = true;
						}

						status.m = -1;
						return true;
					};

					// Locate blocks and their damage
					for (status.n = 0, status.m = -1; ; ++status.n)
					{
						auto cc = line[i + std::size_t(status.n)];

						if (status.m == -1)
						{
							status.m = status.n;
						}
						// Next damage value
						if (cc == ':')
						{
							if (read_block())
								continue;
							return false;
						}

						// Block ID
						if (std::isdigit(cc))
							continue;
						// Whitespaces
						if (string::iswhite(cc))
							break;
						return false;
					}

					if (!read_block())
						return false;

					// Handle no damage
					if (!status.has_damage)
					{
						blockids.push_back(status.block);
					}

					i += std::size_t(status.n);
					++num_spaces;
				}
				// Namespace ID
				else
				{
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

					nsids.push_back(line.substr(i, n - i));
					i += n - i;
					++num_spaces;
				}
			}
			else
			{
				// Try hex value
				if (num_spaces == 0)
				{
					for (decltype(i) n = 0; n < 6; ++n)
					{
						auto cc = line[i + n];
						// Not hex
						if (!std::isxdigit(cc))
						{
							// Not RGB
							if (!std::isdigit(cc) && !string::iswhite(cc))
							{
								// Error: Invalid character
								return false;
							}
							// Is RGB
							color.r = int(std::stoul(line.substr(i, n)));
							i += n;
							++num_spaces;
							break;
						}
					}

					// Is hex, verification success
					if (num_spaces == 0)
					{
						// Alpha
						decltype(i) n;
						for (n = 6; n < 8; ++n)
						{
							auto cc = line[i + n];
							if (string::iswhite(cc))
								break;
							// Not hex
							if (!std::isxdigit(cc))
							{
								// Error: Invalid character
								return false;
							}
						}
						unsigned long hex = std::stoul(line.substr(i, n), nullptr, 16);

						if (n == 6)
						{
							color.r = (hex >> 16) & 0xff;
							color.g = (hex >> 8) & 0xff;
							color.b = (hex >> 0) & 0xff;
						}
						else if (n == 8)
						{
							color.r = (hex >> 24) & 0xff;
							color.g = (hex >> 16) & 0xff;
							color.b = (hex >> 8) & 0xff;
							color.a = (hex >> 0) & 0xff;
						}
						i += n;
					}
				}
				else
				{
					// Verify RGB
					decltype(i) n;
					for (n = 0; n < 3; ++n)
					{
						auto cc = line[i + n];
						if (string::iswhite(cc))
							break;
						// Not RGB
						if (!std::isdigit(cc))
						{
							// Error: Invalid character
							return false;
						}
					}
					auto v = std::stoul(line.substr(i, n));
					i += n-1;

					switch (num_spaces)
					{
					case 1:
						color.g = int(v);
						break;
					case 2:
						color.b = int(v);
						break;
					case 3:
						color.a = int(v);
						break;
					}
				}
			}
		}

		ColorIndex id_index = static_cast<ColorIndex>(colors.size());

		// Finally save everything
		for (auto value : blockids)
		{
			old_indices[value] = id_index;
		}
		for (auto value	: nsids)
		{
			new_indices[value] = id_index;
		}
		colors.emplace_back(color);
	}

	return true;
}

BlockColor::ColorIndex BlockColor::getIndex(uint16_t id) const
{
	auto it = old_indices.find(id);
	if (it == old_indices.end())
	{
		if (id <= 0xFF)
			return colors.size();
		// Flatten data value to retrieve values that may exist
		return getIndex(0xFF & id);
	}
	return it->second;
}

BlockColor::ColorIndex BlockColor::getIndex(const std::string & id) const
{
	auto it = new_indices.find(id);
	if (it == new_indices.end())
		return colors.size();
	return it->second;
}

utility::RGBA BlockColor::getColor(ColorIndex index) const
{
	if (index >= colors.size())
		return utility::RGBA();
	return colors[index];
}

bool BlockColor::hasColors() const
{
	return !colors.empty();
}

