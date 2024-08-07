// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/STDFormat.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Extern/utf8.h"
#include "Pipe/Memory/STLAllocator.h"
#include "PipeArrays.h"
#include "PipeSerializeFwd.h"

#include <string>


#pragma warning(push)
#pragma warning(disable:4996)


namespace p
{
	template<typename CharType>
	using TString =
	    std::basic_string<CharType, std::char_traits<CharType>, std::allocator<CharType>>;
	using String  = TString<char>;
	using WString = TString<WideChar>;

	template<typename... Args>
	using FormatString = std::basic_format_string<char, Args...>;

	namespace Strings
	{
		template<typename StringType = String, typename... Args>
		inline StringType Format(StringView format, Args... args)
		{
			String str;
			std::vformat_to(std::back_inserter(str), format, std::make_format_args(args...));
			return Move(str);
		}

		template<typename StringType, typename... Args>
		inline void FormatTo(StringType& buffer, StringView format, Args... args)
		{
			std::vformat_to(std::back_inserter(buffer), format, std::make_format_args(args...));
		}

		// Format an string using a compile time format
		template<typename StringType = String, typename... Args>
		inline StringType Format(FormatString<Args...> format, Args... args)
		{
			String str;
			std::format_to(std::back_inserter(str), format, p::Forward<Args>(args)...);
			return Move(str);
		}

		// Format into an existing string using a compile time format
		template<typename StringType, typename... Args>
		inline void FormatTo(StringType& buffer, FormatString<Args...> format, Args... args)
		{
			std::format_to(std::back_inserter(buffer), format, p::Forward<Args>(args)...);
		}

		template<typename StringType, typename T>
		inline void ToString(StringType& buffer, T value, FormatString<T> format = "{}")
		{
			std::format_to(std::back_inserter(buffer), format, p::Forward<T>(value));
		}

		template<typename StringType, typename T>
		inline StringType ToString(T value)
		{
			StringType str;
			ToString(str, value);
			return str;
		}

		PIPE_API String ToSentenceCase(StringView value);

		/**
		 * Breaks up a delimited string into elements of a string array.
		 *
		 * @param	InArray		The array to fill with the string pieces
		 * @param	pchDelim	The string to delimit on
		 * @param	InCullEmpty	If 1, empty strings are not added to the array
		 *
		 * @return	The number of elements in InArray
		 */
		PIPE_API i32 ParseIntoArray(const String& str, TArray<String>& OutArray,
		    const char* pchDelim, bool InCullEmpty = true);

		PIPE_API void RemoveFromStart(String& str, sizet size);
		PIPE_API void RemoveFromEnd(String& str, sizet size);
		PIPE_API void RemoveFromEnd(String& str, StringView subStr);

		PIPE_API bool RemoveCharFromEnd(String& str, char c);

		PIPE_API i32 Split(const String& str, TArray<String>& tokens, const char delim);

		PIPE_API bool Split(const String& str, String& a, String& b, const char* delim);

		PIPE_API bool IsNumeric(const String& str);
		PIPE_API bool IsNumeric(const char* Str);

		PIPE_API String ParseMemorySize(sizet size);

		template<typename ToStringType, typename FromChar>
		inline void ConvertTo(TStringView<FromChar> source, ToStringType& dest)
		{
			using ToChar = typename ToStringType::value_type;
			static_assert(
			    std::is_integral_v<FromChar>, "FromChar is not integral (so it is not a char)");
			static_assert(
			    std::is_integral_v<ToChar>, "ToChar is not integral (so it is not a char)");

			if constexpr (IsSame<FromChar, ToChar>)
			{
				dest += source;
			}
			else if constexpr (sizeof(FromChar) == 1 && sizeof(ToChar) == 2)
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
				// TODO: Find a way to assert at compile time except on the previous cases
				// static_assert(false, "Unknown char conversion");
			}
		}

		template<typename ToStringType, typename FromChar>
		inline ToStringType Convert(TStringView<FromChar> source)
		{
			ToStringType dest;
			ConvertTo(source, dest);
			return Move(dest);
		}
		template<typename ToStringType, typename FromChar>
		inline ToStringType Convert(const TString<FromChar>& source)
		{
			ToStringType dest;
			ConvertTo(TStringView<FromChar>{source}, dest);
			return Move(dest);
		}
	};    // namespace Strings


	inline sizet GetHash(const String& str)
	{
		return GetStringHash(str.data());
	}
}    // namespace p

#pragma warning(pop)
