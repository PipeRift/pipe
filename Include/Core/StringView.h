// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Core/Hash.h"
#include "Core/Platform.h"
#include "Templates/Optional.h"

#include <string_view>


namespace Pipe::Core
{
	template<typename CharType>
	using TStringView = std::basic_string_view<CharType, std::char_traits<CharType>>;

	using StringView  = TStringView<TChar>;
	using WStringView = TStringView<WideChar>;

	template<>
	struct Hash<StringView>
	{
		sizet operator()(const StringView& str) const
		{
			return GetStringHash(str.data(), str.size());
		}
	};

	template<>
	struct Hash<const TChar*>
	{
		sizet operator()(const TChar* str) const
		{
			return GetStringHash(str);
		}
	};


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


		// BEGIN TChar API
		// Specializations to avoid requiring <TChar> on function calls

		CORE_API constexpr sizet Find(
		    const StringView str, const TChar c, FindDirection direction = FindDirection::Front)
		{
			return Find<TChar>(str, c, direction);
		}

		CORE_API constexpr sizet Find(const StringView str, const StringView subStr,
		    FindDirection direction = FindDirection::Front)
		{
			return Find<TChar>(str, subStr, direction);
		}

		CORE_API constexpr bool Contains(
		    const StringView str, const TChar c, FindDirection direction = FindDirection::Front)
		{
			return Contains<TChar>(str, c, direction);
		}

		CORE_API constexpr bool Contains(const StringView str, const StringView subStr,
		    FindDirection direction = FindDirection::Front)
		{
			return Contains<TChar>(str, subStr, direction);
		}

		CORE_API constexpr bool Equals(const StringView str, const StringView other)
		{
			return Equals<TChar>(str, other);
		}

		CORE_API constexpr bool Equals(const StringView str, const TChar c)
		{
			return Equals<TChar>(str, c);
		}

		CORE_API constexpr bool StartsWith(const StringView str, const StringView subStr)
		{
			return StartsWith<TChar>(str, subStr);
		}

		CORE_API constexpr bool EndsWith(const StringView str, const StringView subStr)
		{
			return EndsWith<TChar>(str, subStr);
		}

		CORE_API constexpr bool EndsWith(const StringView str, const TChar c)
		{
			return EndsWith<TChar>(str, c);
		}

		CORE_API constexpr StringView RemoveFromStart(const StringView str, sizet size)
		{
			return RemoveFromStart<TChar>(str, size);
		}

		CORE_API constexpr StringView RemoveFromStart(const StringView str, const StringView subStr)
		{
			return RemoveFromStart<TChar>(str, subStr);
		}

		CORE_API constexpr StringView RemoveFromEnd(const StringView str, sizet size)
		{
			return RemoveFromEnd<TChar>(str, size);
		}

		CORE_API constexpr StringView RemoveFromEnd(const StringView str, const StringView subStr)
		{
			return RemoveFromEnd<TChar>(str, subStr);
		}

		CORE_API constexpr StringView FrontSubstr(StringView str, sizet size)
		{
			return FrontSubstr<TChar>(str, size);
		}

		CORE_API constexpr StringView BackSubstr(StringView str, sizet size)
		{
			return BackSubstr<TChar>(str, size);
		}

		CORE_API constexpr void Replace(
		    StringView str, const TChar searchChar, const TChar replacementChar)
		{
			Replace<TChar>(str, searchChar, replacementChar);
		}

		CORE_API constexpr sizet Length(const TChar* str)
		{
			return Length<TChar>(str);
		}

		CORE_API TOptional<u32> ToU32(StringView str);

		CORE_API TOptional<i32> ToI32(StringView str);

		// END TChar API
	}    // namespace Strings
}    // namespace Pipe::Core

namespace Pipe
{
	using namespace Pipe::Core;
}
