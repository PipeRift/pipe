// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/GenericPlatform.h"

#define P_PLATFORM_LINUX_USE_CHAR16 0


namespace p
{
	/**
	 * Linux specific types
	 **/
	struct LinuxPlatformTypes : public GenericPlatformTypes
	{
		using DWORD        = unsigned int;
		using sizet        = __SIZE_TYPE__;
		using TYPE_OF_NULL = decltype(__null);
#if P_PLATFORM_LINUX_USE_CHAR16
		using WideChar = char16_t;
#endif
	};

	using PlatformTypes = LinuxPlatformTypes;
}    // namespace p


#if P_DEBUG
	#define P_FORCEINLINE inline
#else
	#define P_FORCEINLINE inline __attribute__((always_inline))
#endif    // P_DEBUG
#define P_NOINLINE __attribute__((noinline))

#define P_PLATFORM_BREAK() __asm__ volatile("int $0x03")

#if (__clang_major__ > 3 || (__clang_major__ == 3 && __clang_minor__ >= 6))
	#define DISABLE_OPTIMIZATION_ACTUAL _Pragma("clang optimize off")
	#define ENABLE_OPTIMIZATION_ACTUAL _Pragma("clang optimize on")
#endif

#if P_PLATFORM_LINUX_USE_CHAR16
	#undef PLATFORM_TCHAR_IS_CHAR16
	#define PLATFORM_TCHAR_IS_CHAR16 1
#endif
