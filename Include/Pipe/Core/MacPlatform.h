// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/GenericPlatform.h"

#include <cstddef>


#define P_PLATFORM_MACOS_USE_CHAR16 0


namespace p
{
	/**
	 * Mac specific types
	 **/
	struct MacPlatformTypes : public GenericPlatformTypes
	{
		using DWORD        = unsigned int;
		using sizet        = std::size_t;
		using TYPE_OF_NULL = decltype(nullptr);
#if P_PLATFORM_MACOS_USE_CHAR16
		using WideChar = char16_t;
		using TChar    = WideChar;
#else
		using Char16 = char16_t;
#endif
	};

	using PlatformTypes = MacPlatformTypes;
}    // namespace p

#if __is_target_arch(arm64) || __is_target_arch(arm64e)
	#define P_PLATFORM_MACOS_ARM64 1
	#define P_PLATFORM_MACOS_X86 0
#else
	#define P_PLATFORM_MACOS_ARM64 0
	#define P_PLATFORM_MACOS_X86 1
#endif


#if P_DEBUG
	#define P_FORCEINLINE inline /* Don't force code to be inline */
#else
	#define P_FORCEINLINE inline __attribute__((always_inline)) /* Force code to be inline */
#endif
#define P_NOINLINE __attribute__((noinline))

#if P_PLATFORM_MACOS_X86
	#define P_PLATFORM_BREAK() __asm__("int $3")
#else
	#define P_PLATFORM_BREAK() __builtin_debugtrap()
#endif


#if (__clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 6))
	#define DISABLE_OPTIMIZATION_ACTUAL _Pragma("clang optimize off")
	#define ENABLE_OPTIMIZATION_ACTUAL _Pragma("clang optimize on")
#endif

#if P_PLATFORM_MACOS_USE_CHAR16
	#undef PLATFORM_TCHAR_IS_CHAR16
	#define PLATFORM_TCHAR_IS_CHAR16 1
#endif
