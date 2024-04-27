// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#ifndef P_PLATFORM_WINDOWS
	#if defined(_WIN64)
		#define P_PLATFORM_WINDOWS 1
	#elif defined(_WIN32)
		#define P_PLATFORM_WINDOWS 1
	#else
		#define P_PLATFORM_WINDOWS 0
	#endif
#endif
#ifndef P_PLATFORM_LINUX
	#if defined(__linux__)
		#define P_PLATFORM_LINUX 1
	#else
		#define P_PLATFORM_LINUX 0
	#endif
#endif
#ifndef P_PLATFORM_MACOS
	#if defined(__APPLE__)
		#define P_PLATFORM_MACOS 1
	#else
		#define P_PLATFORM_MACOS 0
	#endif
#endif


#if P_PLATFORM_WINDOWS
	#include "Pipe/Core/WindowsPlatform.h"
#elif P_PLATFORM_LINUX
	#include "Pipe/Core/LinuxPlatform.h"
#elif P_PLATFORM_MACOS
	#include "Pipe/Core/MacPlatform.h"
#else
	#error Unknown platform
#endif


namespace p
{
	/////////////////////////////////////////////////////////////
	// Define platform types as global types

	// Unsigned base types
	// An 8-bit unsigned integer
	using u8 = std::uint8_t;
	// A 16-bit unsigned integer
	using u16 = std::uint16_t;
	// A 32-bit unsigned integer
	using u32 = std::uint32_t;
	// A 64-bit unsigned integer
	using u64 = std::uint64_t;

	// Signed base types
	// An 8-bit signed integer
	using i8 = std::int8_t;
	/// A 16-bit signed integer
	using i16 = std::int16_t;
	/// A 32-bit signed integer
	using i32 = std::int32_t;
	/// A 64-bit signed integer
	using i64 = std::int64_t;
	/// A 32-bit floating point number
	using f32 = float;
	/// A 64-bit floating point number
	using f64 = double;

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
}    // namespace p


#if !defined(TX)
	#if PLATFORM_TCHAR_IS_WCHAR
		#define TX(x) L##x
	#elif PLATFORM_TCHAR_IS_CHAR8
		#define TX(x) x
	#elif PLATFORM_TCHAR_IS_CHAR16
		#define TX(x) u##x
	#elif PLATFORM_TCHAR_IS_CHAR32
		#define TX(x) U##x
	#else
		#define TX(x) x
	#endif
#endif

#undef UNIQUE_FUNCTION_ID
#if defined(_MSC_VER)
	#define UNIQUE_FUNCTION_ID __FUNCSIG__
#else
	#if defined(__GNUG__)
		#define UNIQUE_FUNCTION_ID __PRETTY_FUNCTION__
	#endif
#endif

#if P_RELEASE
	#define P_DEBUG_PLATFORM_BREAK()
#else
	#define P_DEBUG_PLATFORM_BREAK() P_PLATFORM_BREAK()
#endif

#define DISABLE_OPTIMIZATION DISABLE_OPTIMIZATION_ACTUAL
#if P_DEBUG
	#define ENABLE_OPTIMIZATION DISABLE_OPTIMIZATION_ACTUAL
#else
	#define ENABLE_OPTIMIZATION ENABLE_OPTIMIZATION_ACTUAL
#endif

#ifndef P_FORCEINLINE
	#if defined(_MSC_VER)
		#define P_FORCEINLINE __forceinline
	#else
		#define P_FORCEINLINE inline __attribute__((always_inline))
	#endif
#endif

#if defined(__cplusplus) && (__cplusplus >= 201703)
	#define P_NODISCARD [[nodiscard]]
#elif (defined(__GNUC__) && (__GNUC__ >= 4)) \
    || defined(__clang__)    // includes clang, icc, and clang-cl
	#define P_NODISCARD __attribute__((warn_unused_result))
#elif defined(_HAS_NODISCARD)
	#define P_NODISCARD _NODISCARD
#elif (_MSC_VER >= 1700)
	#define P_NODISCARD _Check_return_
#else
	#define P_NODISCARD
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
	#if defined(__MINGW32__)
		#define P_RESTRICT
		#define P_ATTR_MALLOC __attribute__((malloc))
	#else
		#if (_MSC_VER >= 1900) && !defined(__EDG__)
			#define P_RESTRICT __declspec(allocator) __declspec(restrict)
		#else
			#define P_RESTRICT __declspec(restrict)
		#endif
		#define P_ATTR_MALLOC
	#endif
#elif defined(__GNUC__)    // includes clang and icc
	#define P_RESTRICT
	#define P_ATTR_MALLOC __attribute__((malloc))
#else
	#define P_RESTRICT
	#define P_ATTR_MALLOC
#endif