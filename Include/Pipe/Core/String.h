// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Reflect/TypeName.h"
#include "Pipe/Serialize/SerializationFwd.h"

#include <fmt/format.h>
#include <utf8.h>

#include <string>


#pragma warning(push)
#pragma warning(disable:4996)


namespace p::core
{
	template<typename CharType, typename Allocator = STLAllocator<CharType>>
	using TString = std::basic_string<CharType, std::char_traits<CharType>, Allocator>;
	using String  = TString<TChar>;
	using WString = TString<WideChar>;


	template<typename CharType, typename Allocator = STLAllocator<CharType>>
	using TStringBuffer = fmt::basic_memory_buffer<CharType, fmt::inline_buffer_size, Allocator>;
	using StringBuffer  = TStringBuffer<TChar>;


	namespace Strings
	{
		template<typename... Args>
		inline String Format(StringView format, Args... args)
		{
			String str;
			fmt::format_to(
			    std::back_inserter(str), fmt::runtime(format), std::forward<Args>(args)...);
			return Move(str);
		}

		template<typename... Args>
		inline void FormatTo(String& buffer, StringView format, Args... args)
		{
			fmt::format_to(
			    std::back_inserter(buffer), fmt::runtime(format), std::forward<Args>(args)...);
		}

		PIPE_API String ToSentenceCase(const String& value);

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
		    const TChar* pchDelim, bool InCullEmpty = true);

		PIPE_API void RemoveFromStart(String& str, sizet size);
		PIPE_API void RemoveFromEnd(String& str, sizet size);

		PIPE_API i32 Split(const String& str, TArray<String>& tokens, const TChar delim);

		PIPE_API bool Split(const String& str, String& a, String& b, const TChar* delim);

		PIPE_API bool IsNumeric(const String& str);
		PIPE_API bool IsNumeric(const TChar* Str);

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
	};    // namespace Strings
}    // namespace p::core

namespace p
{
	using namespace p::core;


	template<>
	struct Hash<String>
	{
		sizet operator()(const String& str) const
		{
			return GetStringHash(str.data());
		}
	};

	PIPE_API void Read(p::Reader& ct, p::String& val);
	PIPE_API void Write(p::Writer& ct, const p::String& val);
	OVERRIDE_TYPE_NAME(String)
}    // namespace p


#pragma warning(pop)
