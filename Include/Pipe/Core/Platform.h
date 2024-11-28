// Copyright 2015-2024 Piperift - All rights reserved

#pragma once


#ifndef P_PLATFORM_WINDOWS
	#if defined(_WIN64) || defined(_WIN32)
		#define P_PLATFORM_WINDOWS 1
	#else
		#define P_PLATFORM_WINDOWS 0
	#endif
#endif
#ifndef P_PLATFORM_LINUX
	#if defined(__linux__)
		#define P_PLATFORM_LINUX 1
		#if defined(__ANDROID__) || defined(ANDROID)
			#define P_PLATFORM_ANDROID 1
		#endif
	#else
		#define P_PLATFORM_LINUX 0
		#define P_PLATFORM_ANDROID 0
	#endif
#endif
#ifndef P_PLATFORM_APPLE
	#if defined(__APPLE__) || defined(__APPLE_CC__)
		#define P_PLATFORM_APPLE 1
		#if defined(__IPHONE__) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE) \
		    || (defined(TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR)
			#define P_PLATFORM_IOS 1
			#define P_PLATFORM_MACOS 0
		#else
			#define P_PLATFORM_MACOS 1
			#define P_PLATFORM_IOS 0
		#endif
	#else
		#define P_PLATFORM_APPLE 0
		#define P_PLATFORM_MACOS 0
		#define P_PLATFORM_IOS 0
	#endif
#endif

#ifndef P_PLATFORM_BSD
	#if defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	    || defined(__DragonFly__)
		#define P_PLATFORM_BSD 1
	#else
		#define P_PLATFORM_BSD 0
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


// Platform break includes
#ifdef _MSC_VER
#elif defined(__i386__) || defined(__x86_64__)
#elif defined(__thumb__)
#elif defined(__arm__) && !defined(__thumb__)
#elif defined(__aarch64__) && defined(__clang__)
#elif defined(__aarch64__)
#elif defined(__powerpc__)
#elif defined(__riscv)
#else
	#include <signal.h>
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

#pragma region Platform Break
namespace p
{
	P_FORCEINLINE static void PlatformBreak()
	{
#ifdef _MSC_VER    // MSVC
		__debugbreak();
#elif defined(__i386__) || defined(__x86_64__)
		__asm__ volatile("int $0x03");
#elif defined(__thumb__)
		// See 'arm-linux-tdep.c' in GDB source, 'eabi_linux_thumb_le_breakpoint'
		__asm__ volatile(".inst 0xde01");
#elif defined(__arm__) && !defined(__thumb__)
		// See 'arm-linux-tdep.c' in GDB source, 'eabi_linux_arm_le_breakpoint'
		__asm__ volatile(".inst 0xe7f001f0");
#elif defined(__aarch64__) && defined(__clang__)
		__builtin_debugtrap();
#elif defined(__aarch64__)
		// See 'aarch64-tdep.c' in GDB source, 'aarch64_default_breakpoint'
		__asm__ volatile(".inst 0xd4200000");
#elif defined(__powerpc__)
		// See 'rs6000-tdep.c' in GDB source, 'rs6000_breakpoint'
		__asm__ volatile(".4byte 0x7d821008");
#elif defined(__riscv)
		// See 'riscv-tdep.c' in GDB source, 'riscv_sw_breakpoint_from_kind'
		__asm__ volatile(".4byte 0x00100073");
#else
		raise(SIGTRAP);
#endif
	}

#if P_DEBUG
	P_FORCEINLINE static void PlatformDebugBreak()
	{
		PlatformBreak();
	}
#else
	P_FORCEINLINE static void PlatformDebugBreak() {}
#endif
}    // namespace p

#pragma endregion Platform Break


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
