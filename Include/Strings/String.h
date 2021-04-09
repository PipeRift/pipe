// Copyright 2015-2021 Piperift - All rights reserved

#pragma once


#include "Containers/Array.h"
#include "Math/Math.h"
#include "Misc/Hash.h"
#include "Misc/Utility.h"
#include "Platform/Platform.h"
#include "Reflection/TypeName.h"
#include "Strings/StringView.h"
#include "Templates/Optional.h"
#include "Templates/Tuples.h"

#include <fmt/chrono.h>
#include <fmt/format.h>
#include <utf8.h>

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


	namespace Strings
	{
		template <typename... Args>
		inline String Format(StringView format, Args... args)
		{
			String str;
			fmt::format_to(std::back_inserter(str), format, std::forward<Args>(args)...);
			return Move(str);
		}

		template <typename... Args>
		inline void FormatTo(String& buffer, StringView format, Args... args)
		{
			fmt::format_to(std::back_inserter(buffer), format, std::forward<Args>(args)...);
		}

		CORE_API void ToSentenceCase(const String& str, String& result);

		/**
		 * Breaks up a delimited string into elements of a string array.
		 *
		 * @param	InArray		The array to fill with the string pieces
		 * @param	pchDelim	The string to delimit on
		 * @param	InCullEmpty	If 1, empty strings are not added to the array
		 *
		 * @return	The number of elements in InArray
		 */
		CORE_API i32 ParseIntoArray(const String& str, TArray<String>& OutArray,
		    const TChar* pchDelim, bool InCullEmpty = true);

		CORE_API void RemoveFromStart(String& str, sizet size);
		CORE_API void RemoveFromEnd(String& str, sizet size);

		CORE_API i32 Split(const String& str, TArray<String>& tokens, const TChar delim);

		CORE_API bool Split(const String& str, String& a, String& b, const TChar* delim);

		CORE_API bool IsNumeric(const String& str);
		CORE_API bool IsNumeric(const TChar* Str);

		CORE_API String ParseMemorySize(sizet size);

		template <typename ToStringType, typename FromChar>
		inline void ConvertTo(TStringView<FromChar> source, ToStringType& dest) requires(
		    !IsSame<FromChar, typename ToStringType::value_type>)
		{
			using ToChar = typename ToStringType::value_type;
			static_assert(
			    std::is_integral_v<FromChar>, "FromChar is not integral (so it is not a char)");
			static_assert(
			    std::is_integral_v<ToChar>, "ToChar is not integral (so it is not a char)");

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
				// static_assert(false, "Unknown char conversion types");
			}
		}

		template <typename ToStringType, typename FromChar>
		inline ToStringType Convert(TStringView<FromChar> source)
		{
			ToStringType dest;
			ConvertTo(source, dest);
			return dest;
		}

		// Do nothing. Converting to same type
		template <typename CharType>
		inline TStringView<CharType> Convert(TStringView<CharType> source)
		{
			return source;
		}
	};    // namespace Strings


	template <>
	struct Hash<String>
	{
		sizet operator()(const String& str) const
		{
			return GetStringHash(str.data());
		}
	};

	OVERRIDE_TYPE_NAME(String)
}    // namespace Rift
