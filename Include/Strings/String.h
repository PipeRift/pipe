// Copyright 2015-2021 Piperift - All rights reserved

#pragma once


#include "Containers/Array.h"
#include "Containers/Tuples.h"
#include "Math/Math.h"
#include "Misc/Hash.h"
#include "Misc/Optional.h"
#include "Misc/Utility.h"
#include "Platform/Platform.h"
#include "Reflection/TypeName.h"
#include "Strings/StringView.h"

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <utf8.h>

#include <charconv>
#include <regex>
#include <string>


#pragma warning(disable : 4996)


namespace Rift
{
	template <typename CharType, typename Allocator = STLAllocator<CharType>>
	using TString = std::basic_string<CharType, std::char_traits<CharType>, Allocator>;
	using String  = TString<TChar>;
	using WString = TString<WideChar>;

	template <typename CharType, typename Allocator = STLAllocator<CharType>>
	using TStringBuffer = fmt::basic_memory_buffer<CharType, fmt::inline_buffer_size, Allocator>;
	using StringBuffer  = TStringBuffer<TChar>;

	using Regex = std::basic_regex<TChar>;


	struct CORE_API CString
	{
	public:
		template <typename... Args>
		static String Format(StringView format, Args... args)
		{
			String str;
			fmt::format_to(std::back_inserter(str), format, std::forward<Args>(args)...);
			return Move(str);
		}

		template <typename... Args>
		static void FormatTo(String& buffer, StringView format, Args... args)
		{
			fmt::format_to(std::back_inserter(buffer), format, std::forward<Args>(args)...);
		}

		static void ToSentenceCase(const String& str, String& result);

		static String ReplaceCopy(
		    const String& original, const TChar searchChar, const TChar replacementChar)
		{
			String result = original;
			Replace(result, searchChar, replacementChar);
			return result;
		}

		static void Replace(String& value, const TChar searchChar, const TChar replacementChar)
		{
			std::replace(value.begin(), value.end(), searchChar, replacementChar);
		}

		static bool Contains(StringView str, const TChar c)
		{
			return str.find(c) != String::npos;
		}

		static bool Contains(StringView str, StringView subStr)
		{
			return str.find(subStr) != String::npos;
		}

		static constexpr bool Equals(StringView one, StringView other)
		{
			return one.size() == other.size() && std::equal(one.begin(), one.end(), other.begin());
		}

		static constexpr bool Equals(StringView str, const TChar c)
		{
			return str.size() == 1 && str[0] == c;
		}

		/**
		 * Breaks up a delimited string into elements of a string array.
		 *
		 * @param	InArray		The array to fill with the string pieces
		 * @param	pchDelim	The string to delimit on
		 * @param	InCullEmpty	If 1, empty strings are not added to the array
		 *
		 * @return	The number of elements in InArray
		 */
		static i32 ParseIntoArray(const String& str, TArray<String>& OutArray,
		    const TChar* pchDelim, bool InCullEmpty = true);

		static constexpr bool StartsWith(StringView str, StringView subStr)
		{
			return str.size() >= subStr.size() &&
			       std::equal(subStr.begin(), subStr.end(), str.begin()) == 0;
		}

		static constexpr bool EndsWith(StringView str, StringView subStr)
		{
			return str.size() >= subStr.size() &&
			       std::equal(subStr.rbegin(), subStr.rend(), str.rbegin());
		}

		static constexpr bool EndsWith(StringView str, const TChar c)
		{
			return str.size() >= 1 && str.back() == c;
		}

		static void RemoveFromStart(String& str, sizet size)
		{
			str.erase(0, size);
		}
		static void RemoveFromEnd(String& str, sizet size)
		{
			str.erase(str.size() - 1 - size, str.size() - 1);
		}

		static i32 Split(const String& str, TArray<String>& tokens, const TChar delim)
		{
			sizet current, previous = 0;
			current = str.find(delim);
			while (current != std::string::npos)
			{
				tokens.Add(str.substr(previous, current - previous));
				previous = current + 1;
				current  = str.find(delim, previous);
			}
			tokens.Add(str.substr(previous, current - previous));
			return tokens.Size();
		}

		static bool Split(const String& str, String& a, String& b, const TChar* delim)
		{
			const sizet pos = str.find(delim);
			if (pos != String::npos)
			{
				a = str.substr(0, pos);
				b = str.substr(pos, str.size() - pos);
				return true;
			}
			return false;
		}

		static String FrontSubstr(const String& str, i32 size)
		{
			return str.substr(0, size);
		}

		static String BackSubstr(const String& str, i32 size);

		static Optional<u32> ToU32(StringView str)
		{
			u32 val;
			if (std::from_chars(str.data(), str.data() + str.size(), val).ec != std::errc())
			{
				return val;
			}
			return {};
		}

		static Optional<i32> ToI32(StringView str)
		{
			i32 val;
			if (std::from_chars(str.data(), str.data() + str.size(), val).ec != std::errc())
			{
				return val;
			}
			return {};
		}

		static Optional<u32> ToU32(const char* str)
		{
			if (str)
			{
				return std::strtoul(str, nullptr, 0);
			}
			return {};
		}

		static Optional<i32> ToI32(const char* str)
		{
			if (str)
			{
				return std::atoi(str);
			}
			return {};
		}

		static bool IsNumeric(const String& str)
		{
			return IsNumeric(str.data());
		}
		static bool IsNumeric(const TChar* Str);

		static String ParseMemorySize(sizet size);

		template<typename CharType>
		static sizet Length(const CharType* str)
		{
			return std::strlen(str);
		}


		template <typename ToStringType, typename FromChar>
		static void ConvertTo(TStringView<FromChar> source, ToStringType& dest) requires(
		    !IsSame<FromChar, ToStringType::value_type>)
		{
			using ToChar = ToStringType::value_type;
			static_assert(std::is_integral_v<FromChar>, "FromChar is not integral (so it is not a char)");
			static_assert(std::is_integral_v<ToChar>, "ToChar is not integral (so it is not a char)");

			if constexpr (sizeof(FromChar) == 1 && sizeof(ToChar) == 2)
			{
				utf8::utf8to16(source.begin(), source.end(), std::back_inserter(dest));
			}
			else if constexpr (sizeof(FromChar) == 2 && sizeof(ToChar) == 1)
			{
				utf8::utf16to8(source.begin(), source.end(), std::back_inserter(dest));
			}
			else if constexpr (sizeof(FromChar) == 1 && sizeof(ToChar) == 4)
			{
				utf8::utf8to32(source.begin(), source.end(), std::back_inserter(dest));
			}
			else if constexpr (sizeof(FromChar) == 4 && sizeof(ToChar) == 1)
			{
				utf8::utf32to8(source.begin(), source.end(), std::back_inserter(dest));
			}
			else
			{
				//static_assert(false, "Unknown char conversion types");
			}
		}

		template <typename ToStringType, typename FromChar>
		static ToStringType Convert(TStringView<FromChar> source)
		{
			ToStringType dest;
			ConvertTo(source, dest);
			return dest;
		}

		// Do nothing. Converting to same type
		template <typename CharType>
		static TStringView<CharType> Convert(TStringView<CharType> source)
		{
			return source;
		}
	};


	template <>
	struct Hash<String>
	{
		sizet operator()(const String& str) const
		{
			return GetStringHash(str.data());
		}
	};

	template <>
	struct Hash<StringView>
	{
		sizet operator()(const StringView& str) const
		{
			return GetStringHash(str.data());
		}
	};

	template <>
	struct Hash<const TChar*>
	{
		sizet operator()(const TChar* str) const
		{
			return GetStringHash(str);
		}
	};


	OVERRIDE_TYPE_NAME(String)
}    // namespace Rift
