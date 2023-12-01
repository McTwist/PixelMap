#include "format/nbt.hpp"

#include "util/endianess.hpp"

#include <stack>
#include <limits>
#include <algorithm>

// Check correct standard
// Note: Or just convert them correctly
static_assert(std::numeric_limits<float>::is_iec559, "Requires IEEE 754");
static_assert(std::numeric_limits<double>::is_iec559, "Requires IEEE 754");

/*
 * Internal functionality
 */
typedef int UndefinedType;
typedef uint8_t ItType;

// Read a type
UndefinedType read_type(ItType *& ptr);
// Read a number
template<typename T, uint32_t bytes = sizeof(T)>
T read_number(ItType *& ptr, NBT::Endianess endian);
// Read a string
NBT::NBTString read_string(ItType *& ptr, NBT::Endianess endian);
// Read a specific data
template<typename T, uint32_t bytes = sizeof(T)>
T read_data(ItType *& ptr);
// Read a list
template<typename T>
NBT::NBTArray<T> read_list(ItType *& ptr, NBT::Endianess endian);
// Transform a list
template<typename T, class InputIt>
void transform_list(InputIt first, InputIt last, NBT::Endianess endian);

// Skip a number
template<typename T, uint32_t bytes = sizeof(T)>
void skip_number(ItType *& ptr);
// Skip a string
void skip_string(ItType *& ptr, NBT::Endianess endian);
// Skip a list
template<typename T>
void skip_list(ItType *& ptr, NBT::Endianess endian);

// Read a tag
template<class T>
bool read_tag(ItType *& ptr, UndefinedType type, NBT::Tag & tag, std::stack<T> & stack, NBT::Endianess endian);
// Read a value
template<class T>
bool read_value(ItType *& ptr, UndefinedType type, NBT::Value & value, std::stack<T> & stack, NBT::Endianess endian);
// Skip a value
template<class T>
bool skip_value(ItType *& ptr, UndefinedType type, NBT::Value & value, std::stack<T> & stack, NBT::Endianess endian);



namespace NBT
{

// Parse data and send it to a visitor
std::ptrdiff_t Reader::parse(std::vector<uint8_t> & data, Visitor & visitor, Endianess endian)
{
	return parse({data.data(), data.size()}, visitor, endian);
}

std::ptrdiff_t Reader::parse(VectorView<uint8_t> data, Visitor & visitor, Endianess endian)
{
	return parse(data, [&visitor](const Tag & tag)
	{
		return visitor.visit(tag);
	}
	,[&visitor](const Value & value)
	{
		return visitor.visit(value);
	}, endian);
}

// Parse data and send it to a pair of functions
std::ptrdiff_t Reader::parse(std::vector<uint8_t> & data, std::function<bool(const Tag &)> tag_visit, std::function<bool(const Value &)> value_visit, Endianess endian)
{
	return parse({data.data(), data.size()}, tag_visit, value_visit, endian);
}

std::ptrdiff_t Reader::parse(VectorView<uint8_t> data, std::function<bool(const Tag &)> tag_visit, std::function<bool(const Value &)> value_visit, Endianess endian)
{
	ItType * ptr = data.data();
	Tag tag(endian);
	Value value(endian);
	bool skip_tag = false;
	size_t skip_depth = 0;

	struct StackData
	{
		UndefinedType tag;
		int32_t list_size; // Negative value is a compound
	};
	std::stack<StackData> stack;

	UndefinedType type;

	// Read first element
	type = read_type(ptr);
	if (type != TAG_Compound)
		return throwError("Invalid start of stream");
	tag.setName(std::move(read_string(ptr, endian)));
	if (!read_value(ptr, type, tag, stack, endian))
		return throwError("Invalid type found");
	skip_tag = tag_visit && tag_visit(tag);

	while (ptr < data.data() + data.size() && !stack.empty())
	{
		// List
		if (!stack.empty() && stack.top().list_size > 0)
		{
			type = stack.top().tag;

			// Primitives in lists
			if (type != TAG_Compound)
				--stack.top().list_size;

			if (skip_tag && skip_depth <= stack.size())
			{
				if (!skip_value(ptr, type, value, stack, endian))
					return throwError("Invalid type found");
			}
			else
			{
				skip_tag = false;
				skip_depth = 0;

				if (!read_value(ptr, type, value, stack, endian))
					return throwError("Invalid type found");
				skip_tag = value_visit && value_visit(value);

				if (skip_tag)
				{
					switch (type)
					{
					case TAG_List:
						if (tag.count() == 0)
						{
							stack.pop();
							skip_tag = false;
							skip_depth = 0;
							break;
						}
					case TAG_Compound:
						skip_depth = stack.size();
						break;
					// May only be able to skip complex types
					default:
						skip_tag = false;
						skip_depth = 0;
						break;
					}
				}
			}

			// Primitives in lists
			if (type != TAG_Compound)
			{
				// End of list
				if (stack.top().list_size == 0)
					stack.pop();
			}
		}
		// Compound
		else
		{
			type = read_type(ptr);

			if (skip_tag && skip_depth <= stack.size())
			{
				if (type != TAG_End)
					skip_string(ptr, endian);
				if (!skip_value(ptr, type, value, stack, endian))
					return throwError("Invalid type found");
			}
			else
			{
				if (!read_tag(ptr, type, tag, stack, endian))
					return throwError("Invalid type found");

				skip_tag = false;
				skip_depth = 0;

				skip_tag = tag_visit && tag_visit(tag);

				if (skip_tag)
				{
					switch (type)
					{
					case TAG_List:
						if (tag.count() == 0)
						{
							skip_tag = false;
							skip_depth = 0;
							break;
						}
					case TAG_Compound:
						skip_depth = stack.size();
						break;
						// May only be able to skip complex types
					default:
						skip_depth = 0;
						skip_tag = false;
						break;
					}
				}
			}

			// Handle lists
			if (type == TAG_End && !stack.empty() && stack.top().list_size > 0)
			{
				--stack.top().list_size;

				// End of list
				if (stack.top().list_size == 0)
					stack.pop();
			}
		}
	}

	if (!stack.empty())
		return throwError("Reached end of stream");

	return std::distance(data.data(), ptr);
}

std::ptrdiff_t Reader::throwError(const std::string & err)
{
	error = err;
	return -1;
}

} // namespace NBT

// Read a type
inline UndefinedType read_type(ItType *&ptr)
{
	return static_cast<UndefinedType>(read_data<int8_t>(ptr));
}

// Read a number
template<typename T, uint32_t bytes>
T read_number(ItType *&ptr, NBT::Endianess endian)
{
	T value{};
	switch (endian)
	{
	case NBT::ENDIAN_BIG: value = endianess::fromBig<T, ItType *, bytes>(ptr); break;
	case NBT::ENDIAN_LITTLE: value = endianess::fromLittle<T, ItType *, bytes>(ptr); break;
	}
	ptr += bytes;
	return value;
}

// Read a string
inline NBT::NBTString read_string(ItType *&ptr, NBT::Endianess endian)
{
	auto len = static_cast<std::size_t>(read_number<int16_t>(ptr, endian));
	NBT::NBTString name(reinterpret_cast<const char *>(ptr), len);
	ptr += len;
	return name;
}

// Read data
template<typename T, uint32_t bytes>
T read_data(ItType *&ptr)
{
	auto value = *reinterpret_cast<const T *>(ptr);
	ptr += bytes;
	return value;
}

// Read a list
template<typename T>
NBT::NBTArray<T> read_list(ItType *&ptr, NBT::Endianess endian)
{
	auto len = static_cast<std::size_t>(read_number<int32_t>(ptr, endian));
	ItType * _ptr = ptr;
	auto p = reinterpret_cast<T *>(_ptr);
	NBT::NBTArray<T> values(p, p + len);
	ptr += sizeof(T) * len;
	return values;
}

// Transform a list
template<typename T, class InputIt>
void transform_list(InputIt first, InputIt last, NBT::Endianess endian)
{
	switch (endian)
	{
	case NBT::ENDIAN_BIG:
		std::transform(first, last, first,
			[](auto & v) { return endianess::fromBig<T>(reinterpret_cast<uint8_t *>(&v)); });
		break;
	case NBT::ENDIAN_LITTLE:
		std::transform(first, last, first,
			[](auto & v) { return endianess::fromLittle<T>(reinterpret_cast<uint8_t *>(&v)); });
		break;
	}
}

// Skip a number
template<typename T, uint32_t bytes>
void skip_number(ItType *&ptr)
{
	ptr += bytes;
}

// Skip a string
inline void skip_string(ItType *&ptr, NBT::Endianess endian)
{
	ptr += read_number<int16_t>(ptr, endian);
}

// Skip a list
template<typename T>
void skip_list(ItType *&ptr, NBT::Endianess endian)
{
	ptr += sizeof(T) * static_cast<std::size_t>(read_number<int32_t>(ptr, endian));
}

// Read a tag
template<class T>
bool read_tag(ItType *&ptr, UndefinedType type, NBT::Tag & tag, std::stack<T> & stack, NBT::Endianess endian)
{
	static const NBT::NBTString empty_name("");
	// Special handling for end tag
	if (type != NBT::TAG_End)
		tag.setName(read_string(ptr, endian));
	else
		tag.setName(empty_name);

	return read_value(ptr, type, tag, stack, endian);
}

// Read a value
template<class T>
bool read_value(ItType *&ptr, UndefinedType type, NBT::Value & value, std::stack<T> & stack, NBT::Endianess endian)
{
	using namespace NBT;
	switch (type)
	{
	case TAG_End:
		value.set();
		stack.pop();
		break;
	case TAG_Byte:
		value.set(read_number<int8_t>(ptr, endian));
		break;
	case TAG_Short:
		value.set(read_number<int16_t>(ptr, endian));
		break;
	case TAG_Int:
		value.set(read_number<int32_t>(ptr, endian));
		break;
	case TAG_Long:
		value.set(read_number<int64_t>(ptr, endian));
		break;
	case TAG_Float:
		value.set(read_number<float>(ptr, endian));
		break;
	case TAG_Double:
		value.set(read_number<double>(ptr, endian));
		break;
	case TAG_Byte_Array:
		value.set(read_list<int8_t>(ptr, endian));
		break;
	case TAG_String:
		value.set(read_string(ptr, endian));
		break;
	case TAG_List:
		{
			auto tag_type = read_type(ptr);
			auto count = read_number<int32_t>(ptr, endian);
			value.set(TAG_List, count);
			// Only add for list with actual values
			if (count > 0)
				stack.push({ tag_type, count });
		}
		break;
	case TAG_Compound:
		value.set(TAG_Compound, 0);
		stack.push({ TAG_Compound, -1 });
		break;
	case TAG_Int_Array:
		value.set(read_list<int32_t>(ptr, endian));
		break;
	case TAG_Long_Array:
		value.set(read_list<int64_t>(ptr, endian));
		break;
	default:
		return false;
	}
	return true;
}

// Skip a value
template<class T>
bool skip_value(ItType *&ptr, UndefinedType type, NBT::Value &, std::stack<T> & stack, NBT::Endianess endian)
{
	using namespace NBT;
	switch (type)
	{
	case TAG_End:
		stack.pop();
		break;
	case TAG_Byte:
		skip_number<int8_t>(ptr);
		break;
	case TAG_Short:
		skip_number<int16_t>(ptr);
		break;
	case TAG_Int:
		skip_number<int32_t>(ptr);
		break;
	case TAG_Long:
		skip_number<int64_t>(ptr);
		break;
	case TAG_Float:
		skip_number<float>(ptr);
		break;
	case TAG_Double:
		skip_number<double>(ptr);
		break;
	case TAG_Byte_Array:
		skip_list<int8_t>(ptr, endian);
		break;
	case TAG_String:
		skip_string(ptr, endian);
		break;
	case TAG_List:
		{
			auto tag_type = read_type(ptr);
			// Optimization for primitives
			switch (tag_type)
			{
			case TAG_Byte:
				skip_list<int8_t>(ptr, endian);
				break;
			case TAG_Short:
				skip_list<int16_t>(ptr, endian);
				break;
			case TAG_Int:
				skip_list<int32_t>(ptr, endian);
				break;
			case TAG_Long:
				skip_list<int64_t>(ptr, endian);
				break;
			case TAG_Float:
				skip_list<float>(ptr, endian);
				break;
			case TAG_Double:
				skip_list<double>(ptr, endian);
				break;
			case TAG_Byte_Array:
			case TAG_String:
			case TAG_List:
			case TAG_Compound:
			case TAG_Int_Array:
			case TAG_Long_Array:
			case TAG_End:
				{
					auto count = read_number<int32_t>(ptr, endian);
					// Only add for list with actual values
					if (count > 0)
						stack.push({ tag_type, count });
				}
				break;
			default:
				return false;
			}
		}
		break;
	case TAG_Compound:
		stack.push({ TAG_Compound, -1 });
		break;
	case TAG_Int_Array:
		skip_list<int32_t>(ptr, endian);
		break;
	case TAG_Long_Array:
		skip_list<int64_t>(ptr, endian);
		break;
	default:
		return false;
	}
	return true;
}


namespace NBT
{

/*
 * Value
 */

void Value::set(Type t, int32_t size)
{
	_type = t;
	store(size);
}
void Value::set()
{
	_type = TAG_End;
	value.reset();
}
void Value::set(int8_t v)
{
	_type = TAG_Byte;
	store(v);
}
void Value::set(int16_t v)
{
	_type = TAG_Short;
	store(v);
}
void Value::set(int32_t v)
{
	_type = TAG_Int;
	store(v);
}
void Value::set(int64_t v)
{
	_type = TAG_Long;
	store(v);
}
void Value::set(float v)
{
	_type = TAG_Float;
	store(v);
}
void Value::set(double v)
{
	_type = TAG_Double;
	store(v);
}
void Value::set(const NBTString &v)
{
	_type = TAG_String;
	store(v);
}
void Value::set(const NBTByteArray &v)
{
	_type = TAG_Byte_Array;
	store(v);
}
void Value::set(const NBTIntArray &v)
{
	_type = TAG_Int_Array;
	store(v);
}
void Value::set(const NBTLongArray &v)
{
	_type = TAG_Long_Array;
	store(v);
}
int32_t Value::count() const
{
	return type() == TAG_List ? get<int32_t>() : 0;
}
bool Value::transformed() const
{
	switch (_type)
	{
	case TAG_Int_Array:
	case TAG_Long_Array:
		return *_transformed;
	default:
		// All other types are transformed per default
		return true;
	}
}

void Value::transform() const
{
	if (transformed())
		return;
	*_transformed = true;
	void * ptr = value.get();
	switch (_type)
	{
	case TAG_Int_Array:
		{
			auto array = reinterpret_cast<NBTIntArray *>(ptr);
			transform_list<int32_t>(array->begin(), array->end(), endian);
		}
		break;
	case TAG_Long_Array:
		{
			auto array = reinterpret_cast<NBTLongArray *>(ptr);
			transform_list<int64_t>(array->begin(), array->end(), endian);
		}
		break;
	default:
		break;
	}
}

/*
 * Tag
 */

void Tag::set()
{
	name.clear();
	Value::set();
}

void Tag::setName(const NBTString &str)
{
	name = str;
}
void Tag::setName(const NBTString &&str)
{
	name = str;
}
const NBTString &Tag::getName() const
{
	return name;
}

bool Tag::isName(const NBTString &str) const
{
	return name.compare(str) == 0;
}

bool Tag::isName(const char *str) const
{
	return name.compare(str) == 0;
}

} // namespace NBT

