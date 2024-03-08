// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Export.h"

#include <cstdint>


namespace p
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
		using Char8 = unsigned char;

		// A 16-bit character type - In-memory only. 16-bit representation. Should
		// really be char16_t but making this the generic option is easier for
		// compilers which don't fully support C++11 yet (i.e. MSVC).
		using Char16 = unsigned short int;

		// A 32-bit character type - In-memory only.  32-bit representation.  Should
		// really be char32_t but making this the generic option is easier for
		// compilers which don't fully support C++11 yet (i.e. MSVC).
		using Char32 = unsigned int;

		// A switchable character - In-memory only.
		// Either AnsiChar or WideChar
		using TChar = AnsiChar;

		// unsigned int the same size as a pointer
		using uPtr = SelectIntPointerType<std::uint32_t, std::uint64_t, sizeof(void*)>::TIntPointer;

		// signed int the same size as a pointer
		using iPtr = SelectIntPointerType<std::int32_t, std::int64_t, sizeof(void*)>::TIntPointer;

		// unsigned int the same size as a pointer
		using sizet = uPtr;

		// signed int the same size as a pointer
		using ssizet = iPtr;

		using TYPE_OF_NULL    = std::int32_t;
		using TYPE_OF_NULLPTR = decltype(nullptr);
	};
}    // namespace p


#ifndef PLATFORM_TCHAR_IS_WCHAR
	#define PLATFORM_TCHAR_IS_WCHAR 0
#endif
#ifndef PLATFORM_TCHAR_IS_CHAR8
	#define PLATFORM_TCHAR_IS_CHAR8 0
#endif
#ifndef PLATFORM_TCHAR_IS_CHAR16
	#define PLATFORM_TCHAR_IS_CHAR16 0
#endif
#ifndef PLATFORM_TCHAR_IS_CHAR32
	#define PLATFORM_TCHAR_IS_CHAR32 0
#endif


#ifndef P_LIKELY
	#if (defined(__clang__) || defined(__GNUC__)) && (P_PLATFORM_LINUX)
		#define P_LIKELY(x) __builtin_expect(!!(x), 1)
	#else
		#define P_LIKELY(x) (x)
	#endif
#endif

#ifndef P_UNLIKELY
	#if (defined(__clang__) || defined(__GNUC__)) && (P_PLATFORM_LINUX)
		#define P_UNLIKELY(x) __builtin_expect(!!(x), 0)
	#else
		#define P_UNLIKELY(x) (x)
	#endif
#endif
