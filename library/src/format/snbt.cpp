#include "format/nbt.hpp"

#include <spdlog/fmt/fmt.h> // fmt

#include <stack>
#include <sstream>
#include <algorithm>

// Replace all occurrences of from with to
static std::string replace_all(NBT::NBTString str, const std::string_view & from, const std::string_view & to);

namespace NBT
{

std::string to_snbt(std::vector<uint8_t> & data, Endianess endian)
{
	return to_snbt({data.data(), data.size()}, endian);
}
std::string to_snbt(VectorView<uint8_t> data, Endianess endian)
{
	// TODO: Add support for ","
	// TODO: Optionally remove newline and indentation
	// TODO: Make quotes optional for tag names
	std::ostringstream out;
	Reader reader;
	std::stack<int32_t> stack; // Negative is compound
	auto indent = [&stack]() { return std::string(stack.size() * 2, ' '); };
	auto diff = reader.parse(data, [&stack, &indent, &out](const Tag & tag) -> bool {
		std::vector<std::string> buffer;
		std::string line;
		if (tag.type() != TAG_End)
			line += indent() + fmt::format("\"{:s}\":", replace_all(tag.getName(), "\"", "\\\""));
		switch (tag.type())
		{
		case TAG_End:
			stack.pop();
			line += indent() + "}";
			break;
		case TAG_Byte:
			line += fmt::format("{:d}b", tag.get<int8_t>());
			break;
		case TAG_Short:
			line += fmt::format("{:d}s", tag.get<int16_t>());
			break;
		case TAG_Int:
			line += fmt::format("{:d}", tag.get<int32_t>());
			break;
		case TAG_Long:
			line += fmt::format("{:d}l", tag.get<int64_t>());
			break;
		case TAG_Float:
			line += fmt::format("{:f}f", tag.get<float>());
			break;
		case TAG_Double:
			line += fmt::format("{:f}d", tag.get<double>());
			break;
		case TAG_Byte_Array:
			std::transform(
				tag.get<NBT::NBTByteArray>().begin(),
				tag.get<NBT::NBTByteArray>().end(),
				std::back_inserter(buffer),
				[](int8_t a) {
					return fmt::format("{:d}d", a);
				});
			line += fmt::format("[{:s}]", fmt::join(buffer, ","));
			break;
		case TAG_String:
			line += fmt::format("\"{:s}\"", replace_all(tag.get<NBT::NBTString>(), "\"", "\\\""));
			break;
		case TAG_List:
			line += "[";
			stack.emplace(tag.count());
			break;
		case TAG_Compound:
			line += "{";
			stack.emplace(-1);
			break;
		case TAG_Int_Array:
			std::transform(
				tag.get<NBT::NBTIntArray>().begin(),
				tag.get<NBT::NBTIntArray>().end(),
				std::back_inserter(buffer),
				[](int32_t a) {
					return fmt::format("{:d}", a);
				});
			line += fmt::format("[{:s}]", fmt::join(buffer, ","));
			break;
		case TAG_Long_Array:
			std::transform(
				tag.get<NBT::NBTLongArray>().begin(),
				tag.get<NBT::NBTLongArray>().end(),
				std::back_inserter(buffer),
				[](int64_t a) {
					return fmt::format("{:d}l", a);
				});
			line += fmt::format("[{:s}]", fmt::join(buffer, ","));
			break;
		}
		if (!stack.empty() && stack.top() == 0)
		{
			stack.pop();
			line += "\n" + indent() + "]";
		}
		out << line << std::endl;
		return false;
	}, [&stack, &indent, &out](const Value & value) -> bool {
		std::vector<std::string> buffer;
		std::string line;
		if (stack.top() > 0)
			--stack.top();
		switch (value.type())
		{
		case TAG_End: // Should not happen
			break;
		case TAG_Byte:
			line += indent() + fmt::format("{:d}b", value.get<int8_t>());
			break;
		case TAG_Short:
			line += indent() + fmt::format("{:d}s", value.get<int16_t>());
			break;
		case TAG_Int:
			line += indent() + fmt::format("{:d}", value.get<int32_t>());
			break;
		case TAG_Long:
			line += indent() + fmt::format("{:d}l", value.get<int64_t>());
			break;
		case TAG_Float:
			line += indent() + fmt::format("{:f}f", value.get<float>());
			break;
		case TAG_Double:
			line += indent() + fmt::format("{:f}d", value.get<double>());
			break;
		case TAG_Byte_Array:
			std::transform(
				value.get<NBT::NBTByteArray>().begin(),
				value.get<NBT::NBTByteArray>().end(),
				std::back_inserter(buffer),
				[](int8_t a) {
					return fmt::format("{:d}d", a);
				});
			line += indent() + fmt::format("[{:s}]", fmt::join(buffer, ","));
			break;
		case TAG_String:
			line += indent() + fmt::format("\"{:s}\"", replace_all(value.get<NBT::NBTString>(), "\"", "\\\""));
			break;
		case TAG_List:
			line += indent() + "[";
			stack.emplace(value.count());
			break;
		case TAG_Compound:
			line += indent() + "{";
			stack.emplace(-1);
			break;
		case TAG_Int_Array:
			std::transform(
				value.get<NBT::NBTIntArray>().begin(),
				value.get<NBT::NBTIntArray>().end(),
				std::back_inserter(buffer),
				[](int32_t a) {
					return fmt::format("{:d}", a);
				});
			line += indent() + fmt::format("{:s}", fmt::join(buffer, ","));
			break;
		case TAG_Long_Array:
			std::transform(
				value.get<NBT::NBTLongArray>().begin(),
				value.get<NBT::NBTLongArray>().end(),
				std::back_inserter(buffer),
				[](int64_t a) {
					return fmt::format("{:d}l", a);
				});
			line += indent() + fmt::format("[{:s}]", fmt::join(buffer, ","));
			break;
		}
		if (value.type() != TAG_End && stack.top() > 0)
			line += ",";
		else if (stack.top() == 0)
		{
			stack.pop();
			line += "\n" + indent() + "]";
		}
		out << line << std::endl;
		return false;
	}, endian);
	if (diff < 0)
		std::cerr << reader.getError() << std::endl;
	return out.str();
}

} // namespace NBT

std::string replace_all(NBT::NBTString str, const std::string_view & from, const std::string_view & to)
{
	std::string _str{str};
	for (auto pos = str.find(from); pos != NBT::NBTString::npos; pos = str.find(from, pos))
	{
		_str.replace(pos, from.length(), to);
		pos += to.length();
	}
	return _str;
}

