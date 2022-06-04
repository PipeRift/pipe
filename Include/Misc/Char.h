// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include <cctype>
#include <cwctype>
#include <locale>


namespace Pipe
{
	/*-----------------------------------------------------------------------------
	    Character type functions.
	-----------------------------------------------------------------------------*/

	/**
	 * Templated literal struct to allow selection of wide or ansi string literals
	 * based on the character type provided, and not on compiler switches.
	 */
	template<typename T>
	struct TLiteral
	{
		static const AnsiChar Select(const AnsiChar ansi, const WideChar)
		{
			return ansi;
		}
		static const AnsiChar* Select(const AnsiChar* ansi, const WideChar*)
		{
			return ansi;
		}
	};

	template<>
	struct TLiteral<WideChar>
	{
		static const WideChar Select(const AnsiChar, const WideChar wide)
		{
			return wide;
		}
		static const WideChar* Select(const AnsiChar*, const WideChar* wide)
		{
			return wide;
		}
	};

#define LITERAL(CharType, StringLiteral) TLiteral<CharType>::Select(StringLiteral, L##StringLiteral)

	/**
	 * TChar
	 * Set of utility functions operating on a single character. The functions
	 * are specialized for AnsiChar and TChar character types. You can use the
	 * typedefs FChar and FCharAnsi for convenience.
	 */

	template<typename T, const unsigned int Size>
	struct TCharHelpersBase
	{
		typedef T CharType;

		static const CharType LineFeed           = L'\x000A';
		static const CharType VerticalTab        = L'\x000B';
		static const CharType FormFeed           = L'\x000C';
		static const CharType CarriageReturn     = L'\x000D';
		static const CharType NextLine           = L'\x0085';
		static const CharType LineSeparator      = L'\x2028';
		static const CharType ParagraphSeparator = L'\x2029';
	};

	template<typename T>
	struct TCharHelpersBase<T, 1>
	{
		using CharType = T;

		static const CharType LineFeed       = '\x000A';
		static const CharType VerticalTab    = '\x000B';
		static const CharType FormFeed       = '\x000C';
		static const CharType CarriageReturn = '\x000D';
		static const CharType NextLine       = '\x0085';
	};

	template<typename T, const unsigned int Size>
	struct LineBreakImplementation
	{
		using CharType = T;
		static inline bool IsLinebreak(CharType c)
		{
			return c == TCharHelpersBase<CharType, Size>::LineFeed
			    || c == TCharHelpersBase<CharType, Size>::VerticalTab
			    || c == TCharHelpersBase<CharType, Size>::FormFeed
			    || c == TCharHelpersBase<CharType, Size>::CarriageReturn
			    || c == TCharHelpersBase<CharType, Size>::NextLine
			    || c == TCharHelpersBase<CharType, Size>::LineSeparator
			    || c == TCharHelpersBase<CharType, Size>::ParagraphSeparator;
		}
	};

	template<typename T>
	struct LineBreakImplementation<T, 1>
	{
		using CharType = T;
		static inline bool IsLinebreak(CharType c)
		{
			return c == TCharHelpersBase<CharType, 1>::LineFeed
			    || c == TCharHelpersBase<CharType, 1>::VerticalTab
			    || c == TCharHelpersBase<CharType, 1>::FormFeed
			    || c == TCharHelpersBase<CharType, 1>::CarriageReturn
			    || c == TCharHelpersBase<CharType, 1>::NextLine;
		}
	};

	template<typename T>
	struct TCharHelpers : public TCharHelpersBase<T, sizeof(T)>
	{
		using CharType = T;

	public:
		static inline CharType ToUpper(CharType c)
		{
			return std::toupper(c, std::locale());
		}
		static inline CharType ToLower(CharType c)
		{
			return std::tolower(c, std::locale());
		}
		static inline bool IsUpper(CharType c)
		{
			return std::isupper(c, std::locale());
		}
		static inline bool IsLower(CharType c);
		static inline bool IsAlpha(CharType c);
		static inline bool IsGraph(CharType c);
		static inline bool IsPrint(CharType c);
		static inline bool IsPunct(CharType c);

		static inline bool IsAlnum(CharType c);
		static inline bool IsDigit(CharType c);
		static inline bool IsOctDigit(CharType c)
		{
			return c >= '0' && c <= '7';
		}
		static inline bool IsHexDigit(CharType c);
		static inline i32 ConvertCharDigitToInt(CharType c)
		{
			return static_cast<i32>(c) - static_cast<i32>('0');
		}
		static inline bool IsWhitespace(CharType c);
		static inline bool IsIdentifier(CharType c)
		{
			return IsAlnum(c) || IsUnderscore(c);
		}
		static inline bool IsUnderscore(CharType c)
		{
			return c == LITERAL(CharType, '_');
		}

		static inline bool IsLinebreak(CharType c)
		{
			return LineBreakImplementation<CharType, sizeof(CharType)>::IsLinebreak(c);
		}

		static inline i32 StrtoI32(const CharType* str, CharType** end, i32 radix);
	};

	using FChar     = TCharHelpers<TChar>;
	using FCharWide = TCharHelpers<WideChar>;
	using FCharAnsi = TCharHelpers<AnsiChar>;

	/*-----------------------------------------------------------------------------
	    WideChar specialized functions
	-----------------------------------------------------------------------------*/
	template<>
	inline bool TCharHelpers<WideChar>::IsUpper(CharType c)
	{
		return ::iswupper(c) != 0;
	}
	template<>
	inline bool TCharHelpers<WideChar>::IsLower(CharType c)
	{
		return ::iswlower(c) != 0;
	}
	template<>
	inline bool TCharHelpers<WideChar>::IsAlpha(CharType c)
	{
		return ::iswalpha(c) != 0;
	}
	template<>
	inline bool TCharHelpers<WideChar>::IsGraph(CharType c)
	{
		return ::iswgraph(c) != 0;
	}
	template<>
	inline bool TCharHelpers<WideChar>::IsPrint(CharType c)
	{
		return ::iswprint(c) != 0;
	}
	template<>
	inline bool TCharHelpers<WideChar>::IsPunct(CharType c)
	{
		return ::iswpunct(c) != 0;
	}
	template<>
	inline bool TCharHelpers<WideChar>::IsAlnum(CharType c)
	{
		return ::iswalnum(c) != 0;
	}
	template<>
	inline bool TCharHelpers<WideChar>::IsDigit(CharType c)
	{
		return ::iswdigit(c) != 0;
	}
	template<>
	inline bool TCharHelpers<WideChar>::IsHexDigit(CharType c)
	{
		return ::iswxdigit(c) != 0;
	}
	template<>
	inline bool TCharHelpers<WideChar>::IsWhitespace(CharType c)
	{
		return ::iswspace(c) != 0;
	}
	template<>
	inline i32 TCharHelpers<WideChar>::StrtoI32(const CharType* str, CharType** end, i32 radix)
	{
		return ::wcstol(str, end, radix);
	}

	/*-----------------------------------------------------------------------------
	    AnsiChar specialized functions
	-----------------------------------------------------------------------------*/
	template<>
	inline bool TCharHelpers<AnsiChar>::IsUpper(CharType Char)
	{
		return ::isupper((unsigned char)Char) != 0;
	}
	template<>
	inline bool TCharHelpers<AnsiChar>::IsLower(CharType Char)
	{
		return ::islower((unsigned char)Char) != 0;
	}
	template<>
	inline bool TCharHelpers<AnsiChar>::IsAlpha(CharType Char)
	{
		return ::isalpha((unsigned char)Char) != 0;
	}
	template<>
	inline bool TCharHelpers<AnsiChar>::IsGraph(CharType Char)
	{
		return ::isgraph((unsigned char)Char) != 0;
	}
	template<>
	inline bool TCharHelpers<AnsiChar>::IsPrint(CharType Char)
	{
		return ::isprint((unsigned char)Char) != 0;
	}
	template<>
	inline bool TCharHelpers<AnsiChar>::IsPunct(CharType Char)
	{
		return ::ispunct((unsigned char)Char) != 0;
	}
	template<>
	inline bool TCharHelpers<AnsiChar>::IsAlnum(CharType Char)
	{
		return ::isalnum((unsigned char)Char) != 0;
	}
	template<>
	inline bool TCharHelpers<AnsiChar>::IsDigit(CharType Char)
	{
		return ::isdigit((unsigned char)Char) != 0;
	}
	template<>
	inline bool TCharHelpers<AnsiChar>::IsHexDigit(CharType Char)
	{
		return ::isxdigit((unsigned char)Char) != 0;
	}
	template<>
	inline bool TCharHelpers<AnsiChar>::IsWhitespace(CharType Char)
	{
		return ::isspace((unsigned char)Char) != 0;
	}
	template<>
	inline i32 TCharHelpers<AnsiChar>::StrtoI32(const CharType* str, CharType** end, i32 radix)
	{
		return ::strtol(str, end, radix);
	}
}    // namespace Pipe
