// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Export.h"


namespace p::core
{
	//---------------------------------------------------------------------
	// Utility for automatically setting up the pointer-sized integer type
	//---------------------------------------------------------------------

	template<typename T32BITS, typename T64BITS, int PointerSize>
	struct SelectIntPointerType
	{
		// nothing here are is it an error if the partial specializations fail
	};

	template<typename T32BITS, typename T64BITS>
	struct SelectIntPointerType<T32BITS, T64BITS, 8>
	{
		typedef T64BITS TIntPointer;    // select the 64 bit type
	};

	template<typename T32BITS, typename T64BITS>
	struct SelectIntPointerType<T32BITS, T64BITS, 4>
	{
		using TIntPointer = T32BITS;    // select the 32 bit type
	};


	/**
	 * Generic types for almost all compilers and platforms
	 */
	struct PIPE_API GenericPlatformTypes
	{
		// Unsigned base types.
		using uint8  = unsigned char;         // 8-bit  unsigned.
		using uint16 = unsigned short int;    // 16-bit unsigned.
		using uint32 = unsigned int;          // 32-bit unsigned.
		using uint64 = unsigned long long;    // 64-bit unsigned.

		// Signed base types.
		using int8  = signed char;         // 8-bit  signed.
		using int16 = signed short int;    // 16-bit signed.
		using int32 = signed int;          // 32-bit signed.
		using int64 = signed long long;    // 64-bit signed.

		using float32 = float;
		using float64 = double;

		// Character types.
		// An ANSI character - 8-bit fixed-width representation of 7-bit characters.
		using AnsiChar = char;

		// A wide character - In-memory only. ?-bit fixed-width representation of
		// the platform's natural wide character set. Could be different sizes on
		// different platforms.
		using WideChar = wchar_t;

		// An 8-bit character type - In-memory only. 8-bit representation. Should
		// really be char8_t but making this the generic option is easier for
		// compilers which don't fully support C++11 yet (i.e. MSVC).
		using Char8 = char8_t;

		// A 16-bit character type - In-memory only. 16-bit representation. Should
		// really be char16_t but making this the generic option is easier for
		// compilers which don't fully support C++11 yet (i.e. MSVC).
		using Char16 = char16_t;

		// A 32-bit character type - In-memory only.  32-bit representation.  Should
		// really be char32_t but making this the generic option is easier for
		// compilers which don't fully support C++11 yet (i.e. MSVC).
		using Char32 = char32_t;

		// A switchable character - In-memory only.
		// Either AnsiChar or WideChar
		using TChar = AnsiChar;

		// unsigned int the same size as a pointer
		using uPtr = SelectIntPointerType<uint32, uint64, sizeof(void*)>::TIntPointer;

		// signed int the same size as a pointer
		using iPtr = SelectIntPointerType<int32, int64, sizeof(void*)>::TIntPointer;

		// unsigned int the same size as a pointer
		using sizet = uPtr;

		// signed int the same size as a pointer
		using ssizet = iPtr;

		using TYPE_OF_NULL    = int32;
		using TYPE_OF_NULLPTR = decltype(nullptr);
	};
}    // namespace p::core

namespace p
{
	using namespace p::core;
}


#ifndef PLATFORM_TCHAR_IS_WCHAR
#	define PLATFORM_TCHAR_IS_WCHAR 0
#endif
#ifndef PLATFORM_TCHAR_IS_CHAR8
#	define PLATFORM_TCHAR_IS_CHAR8 0
#endif
#ifndef PLATFORM_TCHAR_IS_CHAR16
#	define PLATFORM_TCHAR_IS_CHAR16 0
#endif
#ifndef PLATFORM_TCHAR_IS_CHAR32
#	define PLATFORM_TCHAR_IS_CHAR32 0
#endif


#ifndef LIKELY
#	if (defined(__clang__) || defined(__GNUC__)) && (P_PLATFORM_LINUX)
#		define LIKELY(x) __builtin_expect(!!(x), 1)
#	else
#		define LIKELY(x) (x)
#	endif
#endif

#ifndef UNLIKELY
#	if (defined(__clang__) || defined(__GNUC__)) && (P_PLATFORM_LINUX)
#		define UNLIKELY(x) __builtin_expect(!!(x), 0)
#	else
#		define UNLIKELY(x) (x)
#	endif
#endif
