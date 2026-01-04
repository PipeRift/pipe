// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Core/StringView.h"


namespace p::Strings
{
	template<typename T>
	TOptional<T> InternalToNumber(StringView str)
	{
		T val;
		if (std::from_chars(str.data(), str.data() + str.size(), val).ec == std::errc{})
		{
			return val;
		}
		return {};
	}

	template<>
	TOptional<float> ToNumber<float>(StringView str)
	{
		return InternalToNumber<float>(str);
	}
	template<>
	TOptional<double> ToNumber<double>(StringView str)
	{
		return InternalToNumber<double>(str);
	}
	template<>
	TOptional<u8> ToNumber<u8>(StringView str)
	{
		return InternalToNumber<u8>(str);
	}
	template<>
	TOptional<i8> ToNumber<i8>(StringView str)
	{
		return InternalToNumber<i8>(str);
	}
	template<>
	TOptional<u16> ToNumber<u16>(StringView str)
	{
		return InternalToNumber<u16>(str);
	}
	template<>
	TOptional<i16> ToNumber<i16>(StringView str)
	{
		return InternalToNumber<i16>(str);
	}
	template<>
	TOptional<u32> ToNumber<u32>(StringView str)
	{
		return InternalToNumber<u32>(str);
	}
	template<>
	TOptional<i32> ToNumber<i32>(StringView str)
	{
		return InternalToNumber<i32>(str);
	}
	template<>
	TOptional<u64> ToNumber<u64>(StringView str)
	{
		return InternalToNumber<u64>(str);
	}
	template<>
	TOptional<i64> ToNumber<i64>(StringView str)
	{
		return InternalToNumber<i64>(str);
	}

	TOptional<float> ToFloat(StringView str)
	{
		return InternalToNumber<float>(str);
	}

	TOptional<double> ToDouble(StringView str)
	{
		return InternalToNumber<double>(str);
	}
	TOptional<u8> ToU8(StringView str)
	{
		return InternalToNumber<u8>(str);
	}
	TOptional<i8> ToI8(StringView str)
	{
		return InternalToNumber<i8>(str);
	}
	TOptional<u16> ToU16(StringView str)
	{
		return InternalToNumber<u16>(str);
	}
	TOptional<i16> ToI16(StringView str)
	{
		return InternalToNumber<i16>(str);
	}
	TOptional<u32> ToU32(StringView str)
	{
		return InternalToNumber<u32>(str);
	}
	TOptional<i32> ToI32(StringView str)
	{
		return InternalToNumber<i32>(str);
	}
	TOptional<u64> ToU64(StringView str)
	{
		return InternalToNumber<u64>(str);
	}
	TOptional<i64> ToI64(StringView str)
	{
		return InternalToNumber<i64>(str);
	}
}    // namespace p::Strings
