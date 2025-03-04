// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Optional.h"
#include "PipePlatform.h"

#include <cstring>
#include <string_view>


namespace p
{
	template<typename CharType>
	using TStringView = std::basic_string_view<CharType, std::char_traits<CharType>>;

	using StringView  = TStringView<char>;
	using WStringView = TStringView<WideChar>;


	enum class FindDirection : u8
	{
		Front,
		Back
	};

	/**
	 * Strings library
	 * Provides helpers for common StringView based operations
	 */
	namespace Strings
	{
		// BEGIN Any Char API
		template<typename CharType>
		constexpr sizet Find(const TStringView<CharType> str, const CharType c,
		    FindDirection direction = FindDirection::Front)
		{
			if (direction == FindDirection::Front)
			{
				return str.find(c);
			}
			return str.rfind(c);
		}

		template<typename CharType>
		constexpr sizet Find(const TStringView<CharType> str, const TStringView<CharType> subStr,
		    FindDirection direction = FindDirection::Front)
		{
			if (direction == FindDirection::Front)
			{
				return str.find(subStr);
			}
			return str.rfind(subStr);
		}

		template<typename CharType>
		constexpr bool Contains(const TStringView<CharType> str, const CharType c,
		    FindDirection direction = FindDirection::Front)
		{
			return Find(str, c, direction) != TStringView<CharType>::npos;
		}

		template<typename CharType>
		constexpr bool Contains(const TStringView<CharType> str, const TStringView<CharType> subStr,
		    FindDirection direction = FindDirection::Front)
		{
			return Find(str, subStr, direction) != TStringView<CharType>::npos;
		}

		template<typename CharType>
		constexpr bool Equals(const TStringView<CharType> str, const TStringView<CharType> other)
		{
			return str.size() == other.size() && std::equal(str.begin(), str.end(), other.begin());
		}

		template<typename CharType>
		constexpr bool Equals(const TStringView<CharType> str, const CharType c)
		{
			return str.size() == 1 && str[0] == c;
		}

		template<typename CharType>
		constexpr bool StartsWith(
		    const TStringView<CharType> str, const TStringView<CharType> subStr)
		{
			return str.substr(0, subStr.size()) == subStr;
		}

		template<typename CharType>
		constexpr bool EndsWith(const TStringView<CharType> str, const TStringView<CharType> subStr)
		{
			return str.size() >= subStr.size()
			    && str.compare(str.size() - subStr.size(), StringView::npos, subStr) == 0;
		}

		template<typename CharType>
		constexpr bool EndsWith(const TStringView<CharType> str, const CharType c)
		{
			return str.size() >= 1 && str.back() == c;
		}

		template<typename CharType>
		constexpr TStringView<CharType> RemoveFromStart(const TStringView<CharType> str, sizet size)
		{
			return {str.data() + size, str.size() - size};
		}

		template<typename CharType>
		constexpr TStringView<CharType> RemoveFromStart(
		    const TStringView<CharType> str, const TStringView<CharType> subStr)
		{
			if (StartsWith(str, subStr))
			{
				return RemoveFromStart(str, subStr.size());
			}
			return str;
		}

		template<typename CharType>
		constexpr TStringView<CharType> RemoveFromEnd(const TStringView<CharType> str, sizet size)
		{
			return {str.data(), str.size() - size};
		}

		template<typename CharType>
		constexpr TStringView<CharType> RemoveFromEnd(
		    const TStringView<CharType> str, const TStringView<CharType> subStr)
		{
			if (EndsWith(str, subStr))
			{
				return RemoveFromEnd(str, subStr.size());
			}
			return str;
		}

		template<typename CharType>
		constexpr TStringView<CharType> RemoveCharFromEnd(
		    const TStringView<CharType> str, CharType c)
		{
			if (EndsWith(str, c))
			{
				return RemoveFromEnd(str, 1);
			}
			return str;
		}

		template<typename CharType>
		constexpr TStringView<CharType> FrontSubstr(TStringView<CharType> str, sizet size)
		{
			if (str.size() <= size)
			{
				return str;
			}
			return {str.data(), size};
		}

		template<typename CharType>
		constexpr TStringView<CharType> BackSubstr(TStringView<CharType> str, sizet size)
		{
			if (str.size() <= size)
			{
				return str;
			}
			return {str.data() + (str.size() - size), size};
		}

		template<typename CharType>
		constexpr void Replace(
		    TStringView<CharType> str, const CharType searchChar, const CharType replacementChar)
		{
			for (u32 i = 0; i < str.size(); ++i)
			{
				CharType& c = const_cast<CharType&>(str[i]);
				if (c == searchChar)
				{
					c = replacementChar;
				}
			}
		}

		template<typename CharType>
		constexpr sizet Length(const CharType* str)
		{
			return std::strlen(str);
		}
		// END Any Char API


		// BEGIN char API
		// Specializations to avoid requiring <char> on function calls

		P_API constexpr sizet Find(
		    const StringView str, const char c, FindDirection direction = FindDirection::Front)
		{
			return Find<char>(str, c, direction);
		}

		P_API constexpr sizet Find(const StringView str, const StringView subStr,
		    FindDirection direction = FindDirection::Front)
		{
			return Find<char>(str, subStr, direction);
		}

		P_API constexpr bool Contains(
		    const StringView str, const char c, FindDirection direction = FindDirection::Front)
		{
			return Contains<char>(str, c, direction);
		}

		P_API constexpr bool Contains(const StringView str, const StringView subStr,
		    FindDirection direction = FindDirection::Front)
		{
			return Contains<char>(str, subStr, direction);
		}

		P_API constexpr bool Equals(const StringView str, const StringView other)
		{
			return Equals<char>(str, other);
		}

		P_API constexpr bool Equals(const StringView str, const char c)
		{
			return Equals<char>(str, c);
		}

		P_API constexpr bool StartsWith(const StringView str, const StringView subStr)
		{
			return StartsWith<char>(str, subStr);
		}

		P_API constexpr bool EndsWith(const StringView str, const StringView subStr)
		{
			return EndsWith<char>(str, subStr);
		}

		P_API constexpr bool EndsWith(const StringView str, const char c)
		{
			return EndsWith<char>(str, c);
		}

		P_API constexpr StringView RemoveFromStart(const StringView str, sizet size)
		{
			return RemoveFromStart<char>(str, size);
		}

		P_API constexpr StringView RemoveFromStart(const StringView str, const StringView subStr)
		{
			return RemoveFromStart<char>(str, subStr);
		}


		P_API constexpr StringView RemoveFromEnd(const StringView str, sizet size)
		{
			return RemoveFromEnd<char>(str, size);
		}

		P_API constexpr StringView RemoveFromEnd(const StringView str, const StringView subStr)
		{
			return RemoveFromEnd<char>(str, subStr);
		}

		P_API constexpr StringView RemoveCharFromEnd(const StringView str, const char c)
		{
			return RemoveCharFromEnd<char>(str, c);
		}

		P_API constexpr StringView FrontSubstr(StringView str, sizet size)
		{
			return FrontSubstr<char>(str, size);
		}

		P_API constexpr StringView BackSubstr(StringView str, sizet size)
		{
			return BackSubstr<char>(str, size);
		}

		P_API constexpr void Replace(
		    StringView str, const char searchChar, const char replacementChar)
		{
			Replace<char>(str, searchChar, replacementChar);
		}

		P_API constexpr sizet Length(const char* str)
		{
			return Length<char>(str);
		}

		template<Number T>
		TOptional<T> ToNumber(StringView str);

		P_API TOptional<float> ToFloat(StringView str);
		P_API TOptional<double> ToDouble(StringView str);
		P_API TOptional<u32> ToU32(StringView str);
		P_API TOptional<i32> ToI32(StringView str);
		P_API TOptional<u64> ToU64(StringView str);
		P_API TOptional<i64> ToI64(StringView str);

		/** Begin ToNumber spetializations */
		template<>
		P_API TOptional<float> ToNumber<float>(StringView str);
		template<>
		P_API TOptional<double> ToNumber<double>(StringView str);
		template<>
		P_API TOptional<u8> ToNumber<u8>(StringView str);
		template<>
		P_API TOptional<i8> ToNumber<i8>(StringView str);
		template<>
		P_API TOptional<u16> ToNumber<u16>(StringView str);
		template<>
		P_API TOptional<i16> ToNumber<i16>(StringView str);
		template<>
		P_API TOptional<u32> ToNumber<u32>(StringView str);
		template<>
		P_API TOptional<i32> ToNumber<i32>(StringView str);
		template<>
		P_API TOptional<u64> ToNumber<u64>(StringView str);
		template<>
		P_API TOptional<i64> ToNumber<i64>(StringView str);
		/** End ToNumber spetializations */

		// END char API
	}    // namespace Strings


	inline sizet GetHash(const StringView& str)
	{
		return GetStringHash(str.data(), str.size());
	};

	inline sizet GetHash(const char* str)
	{
		return GetStringHash(str);
	};
}    // namespace p
