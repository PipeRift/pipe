// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Generic/GenericPlatform.h"

#define PLATFORM_LINUX_USE_CHAR16 0


namespace pipe::core
{
	/**
	 * Linux specific types
	 **/
	struct LinuxPlatformTypes : public GenericPlatformTypes
	{
		using DWORD        = unsigned int;
		using sizet        = __SIZE_TYPE__;
		using TYPE_OF_NULL = decltype(__null);
#if PLATFORM_LINUX_USE_CHAR16
		using WideChar = char16_t;
		using TChar    = WideChar;
#endif
	};

	using PlatformTypes = LinuxPlatformTypes;
}    // namespace pipe::core

namespace pipe
{
	using namespace pipe::core;
}


#if BUILD_DEBUG
#	define FORCEINLINE inline
#else
#	define FORCEINLINE inline __attribute__((always_inline))
#endif    // BUILD_DEBUG
#define NOINLINE __attribute__((noinline))

#define PLATFORM_BREAK() __asm__ volatile("int $0x03")

#if PLATFORM_LINUX_USE_CHAR16
#	undef PLATFORM_TCHAR_IS_CHAR16
#	define PLATFORM_TCHAR_IS_CHAR16 1
#endif
