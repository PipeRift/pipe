// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Misc/Hash.h"
#include "Platform/Platform.h"
#include "Templates/Optional.h"

#include <string_view>


namespace Rift
{
	template <typename CharType>
	using TStringView = std::basic_string_view<CharType, std::char_traits<CharType>>;

	using StringView  = TStringView<TChar>;
	using WStringView = TStringView<WideChar>;

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
		CORE_API constexpr sizet Find(
		    const StringView str, const TChar c, FindDirection direction = FindDirection::Front)
		{
			if (direction == FindDirection::Front)
			{
				return str.find(c);
			}
			return str.rfind(c);
		}

		CORE_API constexpr sizet Find(const StringView str, const StringView subStr,
		    FindDirection direction = FindDirection::Front)
		{
			if (direction == FindDirection::Front)
			{
				return str.find(subStr);
			}
			return str.rfind(subStr);
		}

		CORE_API constexpr bool Contains(
		    const StringView str, const TChar c, FindDirection direction = FindDirection::Front)
		{
			return Find(str, c, direction) != StringView::npos;
		}

		CORE_API constexpr bool Contains(const StringView str, const StringView subStr,
		    FindDirection direction = FindDirection::Front)
		{
			return Find(str, subStr, direction) != StringView::npos;
		}

		CORE_API constexpr bool Equals(const StringView str, const StringView other)
		{
			return str.size() == other.size() && std::equal(str.begin(), str.end(), other.begin());
		}

		CORE_API constexpr bool Equals(const StringView str, const TChar c)
		{
			return str.size() == 1 && str[0] == c;
		}

		CORE_API constexpr bool StartsWith(const StringView str, const StringView subStr)
		{
			return str.substr(0, subStr.size()) == subStr;
		}

		CORE_API constexpr bool EndsWith(const StringView str, const StringView subStr)
		{
			return str.size() >= subStr.size() &&
			       str.compare(str.size() - subStr.size(), StringView::npos, subStr) == 0;
		}

		CORE_API constexpr bool EndsWith(const StringView str, const TChar c)
		{
			return str.size() >= 1 && str.back() == c;
		}

		CORE_API constexpr StringView RemoveFromStart(const StringView str, sizet size)
		{
			return {str.data() + size, str.size() - size};
		}

		CORE_API constexpr StringView RemoveFromEnd(const StringView str, sizet size)
		{
			return {str.data(), str.size() - size};
		}

		CORE_API constexpr StringView RemoveFromStart(const StringView str, const StringView subStr)
		{
			if (StartsWith(str, subStr))
			{
				return RemoveFromStart(str, subStr.size());
			}
			return str;
		}

		CORE_API constexpr StringView RemoveFromEnd(const StringView str, const StringView subStr)
		{
			if (EndsWith(str, subStr))
			{
				return RemoveFromEnd(str, subStr.size());
			}
			return str;
		}

		CORE_API constexpr StringView FrontSubstr(StringView str, sizet size)
		{
			if (str.size() <= size)
			{
				return str;
			}
			return {str.data(), size};
		}

		CORE_API constexpr StringView BackSubstr(StringView str, sizet size)
		{
			if (str.size() <= size)
			{
				return str;
			}
			return {str.data() + (str.size() - size), size};
		}

		CORE_API constexpr void Replace(
		    StringView str, const TChar searchChar, const TChar replacementChar)
		{
			for (u32 i = 0; i < str.size(); ++i)
			{
				TChar& c = const_cast<TChar&>(str[i]);
				if (c == searchChar)
				{
					c = replacementChar;
				}
			}
		}

		template <typename CharType>
		constexpr sizet Length(const CharType* str)
		{
			return std::strlen(str);
		}

		CORE_API TOptional<u32> ToU32(StringView str);

		CORE_API TOptional<i32> ToI32(StringView str);
	}    // namespace Strings
}    // namespace Rift
