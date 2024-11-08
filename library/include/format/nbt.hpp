#pragma once
#ifndef NBT_HPP
#define NBT_HPP

#include "vectorview.hpp"

#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <functional>
#include <variant>

// Experimental features
#define USE_STRING_VIEW
#define USE_VECTOR_VIEW
#define USE_VARIANT
#define USE_SMART_ALLOCATION

namespace NBT
{

	enum class Endianess : uint8_t
	{
		BIG = 0,
		LITTLE = 1,
	};

	// The tag type
	enum Type : uint8_t
	{
		TAG_End,
		TAG_Byte,
		TAG_Boolean = TAG_Byte,
		TAG_Short,
		TAG_Int,
		TAG_Long,
		TAG_Float,
		TAG_Double,
		TAG_Byte_Array,
		TAG_String,
		TAG_List,
		TAG_Compound,
		TAG_Int_Array,
		TAG_Long_Array
	};

#ifdef USE_STRING_VIEW
	using NBTString = std::string_view;
#else
	using NBTString = std::string;
#endif
#ifdef USE_VECTOR_VIEW
	template<typename T>
	using NBTArray = VectorView<T>;
#else
	template<typename T>
	using NBTArray = std::vector<T>;
#endif
	using NBTByteArray = NBTArray<int8_t>;
	using NBTIntArray = NBTArray<int32_t>;
	using NBTLongArray = NBTArray<int64_t>;

	// A value of a tag
	class Value
	{
	public:
		Value(Endianess _endian) : endian(_endian) {}
		// Getting value
		template<typename T>
		operator T() const { return get<T>(); }

		// Setting value and type
		void set(Type t, int32_t size);
		void set();
		void set(int8_t v);
		void set(int16_t v);
		void set(int32_t v);
		void set(int64_t v);
		void set(float v);
		void set(double v);
		void set(const NBTString & v);
		void set(const NBTByteArray & v);
		void set(const NBTIntArray & v);
		void set(const NBTLongArray & v);

		// Get value manually
		template<typename T>
		const T & get() const
		{
			transform();
			#ifdef USE_VARIANT
			return std::get<T>(*value);
			#else
			return *reinterpret_cast<T *>(value.get());
			#endif
		}

		// List size
		int32_t count() const;
		// Find out if the value is transformed
		bool transformed() const;

		// Get the type
		inline Type type() const { return _type; }
		// Compare the type
		inline bool operator==(Type t) const { return type() == t; }

	private:
		#ifdef USE_VARIANT
		using NBTType = std::variant<
			int8_t,
			int16_t,
			int32_t,
			int64_t,
			float,
			double,
			NBTString,
			NBTByteArray,
			NBTIntArray,
			NBTLongArray>;
		std::shared_ptr<NBTType> value;
		#else
		std::shared_ptr<void> value;
		#endif
		Type _type = TAG_End;
		Endianess endian;
		std::shared_ptr<bool> _transformed = std::make_shared<bool>(false);

		template<typename T>
		void store(T v)
		{
			#ifdef USE_SMART_ALLOCATION
			if (value.get())
				*value = v;
			else
			#endif
			#ifdef USE_VARIANT
			value = std::make_shared<NBTType>(v);
			#else
			value = std::make_shared<T>(v);
			#endif
			#ifdef USE_SMART_ALLOCATION
			*_transformed = false;
			#else
			_transformed = std::make_shared<bool>(false);
			#endif
		}
		// Convert to correct endianess
		void transform() const;
	};

	// A tag with a name
	class Tag : public Value
	{
	public:
		Tag(Endianess endian) : Value(endian) {}
		// Special set for End tag
		void set();

		// Set and get name
		void setName(const NBTString & str);
		void setName(const NBTString && str);
		const NBTString & getName() const;

		// Quick way to check the name
		bool isName(const NBTString & str) const;
		bool isName(const char * str) const;

	private:
		NBTString name;
	};

	// Easier output for tag objects
	inline std::ostream & operator<<(std::ostream & os, const Value & value)
	{
		switch (value.type())
		{
		case TAG_End: return os << "";
		case TAG_Byte: return os << static_cast<int16_t>(value.get<int8_t>());
		case TAG_Short: return os << value.get<int16_t>();
		case TAG_Int: return os << value.get<int32_t>();
		case TAG_Long: return os << value.get<int64_t>();
		case TAG_Float: return os << value.get<float>();
		case TAG_Double: return os << value.get<double>();
		case TAG_String: return os << value.get<NBT::NBTString>();
		case TAG_Byte_Array: return os << "Array[" << value.get<NBTByteArray>().size() << "]";
		case TAG_Int_Array: return os << "Array[" << value.get<NBTIntArray>().size() << "]";
		case TAG_Long_Array: return os << "Array[" << value.get<NBTLongArray>().size() << "]";
		case TAG_List: return os << "List[" << value.count() << "]";
		case TAG_Compound: return os << "Compound";
		}
		return os << "Unknown";
	}

	// Visitor to retrieve all content
	class Visitor
	{
	protected:
		virtual ~Visitor() = default;
	public:
		// Returning true will skip all structures below the current tag
		// Visit a value from a list
		virtual bool visit(const Value &) = 0;
		// Visit a tag
		virtual bool visit(const Tag &) = 0;
	};

	// Reads all content for NBT
	class Reader
	{
	public:
		// Parse the data with a visitor
		std::ptrdiff_t parse(std::vector<uint8_t> & data, Visitor & visitor, Endianess endian);
		std::ptrdiff_t parse(VectorView<uint8_t> data, Visitor & visitor, Endianess endian);

		// Parse the data with a pair of functions
		std::ptrdiff_t parse(std::vector<uint8_t> & data, std::function<bool(const Tag &)> tag, std::function<bool(const Value &)> value, Endianess endian);
		std::ptrdiff_t parse(VectorView<uint8_t> data, std::function<bool(const Tag &)> tag, std::function<bool(const Value &)> value, Endianess endian);

		// Get previous error as a string
		const std::string & getError() const { return error; }

	private:
		std::string error;

		// Throw an error
		std::ptrdiff_t throwError(const std::string & err);
	};

	// Print NBT structure as SNBT
	std::string to_snbt(VectorView<uint8_t> data, Endianess endian);
	std::string to_snbt(std::vector<uint8_t> & data, Endianess endian);

} // namespace NBT

#endif // NBT_HPP
