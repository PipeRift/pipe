// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#ifndef P_PLATFORM_WINDOWS
#	if defined(_WIN64)
#		define P_PLATFORM_WINDOWS 1
#	elif defined(_WIN32)
#		define P_PLATFORM_WINDOWS 1
#	endif
#endif
#ifndef P_PLATFORM_LINUX
#	if defined(__linux__)
#		define P_PLATFORM_LINUX 1
#	endif
#endif
#ifndef P_PLATFORM_MACOS
#	if defined(__APPLE__)
#		define P_PLATFORM_MACOS 1
#	endif
#endif


#if P_PLATFORM_WINDOWS
#	include "Pipe/Core/Windows/WindowsPlatform.h"
#elif P_PLATFORM_LINUX
#	include "Pipe/Core/Linux/LinuxPlatform.h"
#elif P_PLATFORM_MACOS
#	include "Pipe/Core/Mac/MacPlatform.h"
#else
#	error Unknown platform
#endif


namespace p::core
{
	/////////////////////////////////////////////////////////////
	// Define platform types as global types

	// Unsigned base types

	// An 8-bit unsigned integer
	using u8 = PlatformTypes::uint8;
	// A 16-bit unsigned integer
	using u16 = PlatformTypes::uint16;
	// A 32-bit unsigned integer
	using u32 = PlatformTypes::uint32;
	// A 64-bit unsigned integer
	using u64 = PlatformTypes::uint64;


	// Signed base types

	// An 8-bit signed integer
	using i8 = PlatformTypes::int8;
	/// A 16-bit signed integer
	using i16 = PlatformTypes::int16;
	/// A 32-bit signed integer
	using i32 = PlatformTypes::int32;
	/// A 64-bit signed integer
	using i64 = PlatformTypes::int64;

	/// A 32-bit floating point number
	using f32 = PlatformTypes::float32;
	/// A 64-bit floating point number
	using f64 = PlatformTypes::float64;

	//  Character types

	// An ANSI character. Normally a signed type.
	using AnsiChar = PlatformTypes::AnsiChar;
	// A wide character. Normally a signed type.
	using WideChar = PlatformTypes::WideChar;
	// Either AnsiChar or WideChar, depending on whether the platform supports wide characters
	using TChar = PlatformTypes::TChar;
	// An 8-bit character containing a UTF8 (Unicode, 8-bit, variable-width) code unit.
	using Char8 = PlatformTypes::Char8;
	// A 16-bit character containing a UTF16 (Unicode, 16-bit, variable-width) code unit.
	using Char16 = PlatformTypes::Char16;
	// A 32-bit character containing a UTF32 (Unicode, 32-bit, fixed-width) code unit.
	using Char32 = PlatformTypes::Char32;


	//  Pointer types

	// An unsigned integer the same size as a pointer
	using uPtr = PlatformTypes::uPtr;
	// A signed integer the same size as a pointer
	using iPtr = PlatformTypes::iPtr;
	// An unsigned integer the same size as a pointer, the same as UPTRINT
	using sizet = PlatformTypes::sizet;
	// An integer the same size as a pointer, the same as PTRINT
	using ssizet = PlatformTypes::ssizet;

	// The type of the NULL constant.
	using TYPE_OF_NULL = PlatformTypes::TYPE_OF_NULL;
	// The type of the C++ nullptr keyword.
	using TYPE_OF_NULLPTR = PlatformTypes::TYPE_OF_NULLPTR;


	constexpr i32 NO_INDEX = -1;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}


#if !defined(TX)
#	if PLATFORM_TCHAR_IS_WCHAR
#		define TX(x) L##x
#	elif PLATFORM_TCHAR_IS_CHAR8
#		define TX(x) x
#	elif PLATFORM_TCHAR_IS_CHAR16
#		define TX(x) u##x
#	elif PLATFORM_TCHAR_IS_CHAR32
#		define TX(x) U##x
#	else
#		define TX(x) x
#	endif
#endif

#undef UNIQUE_FUNCTION_ID
#if defined(_MSC_VER)
#	define UNIQUE_FUNCTION_ID __FUNCSIG__
#else
#	if defined(__GNUG__)
#		define UNIQUE_FUNCTION_ID __PRETTY_FUNCTION__
#	endif
#endif

#if P_RELEASE
#	define DEBUG_PLATFORM_BREAK()
#else
#	define DEBUG_PLATFORM_BREAK() PLATFORM_BREAK()
#endif

#define DISABLE_OPTIMIZATION DISABLE_OPTIMIZATION_ACTUAL
#if P_DEBUG
#	define ENABLE_OPTIMIZATION DISABLE_OPTIMIZATION_ACTUAL
#else
#	define ENABLE_OPTIMIZATION ENABLE_OPTIMIZATION_ACTUAL
#endif

#ifndef P_FORCEINLINE
#	if defined(_MSC_VER)
#		define P_FORCEINLINE __forceinline
#	else
#		define P_FORCEINLINE inline __attribute__((always_inline))
#	endif
#endif
