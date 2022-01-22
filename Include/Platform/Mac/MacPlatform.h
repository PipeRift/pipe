// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Platform/Generic/GenericPlatform.h"

#include <cstddef>


#define PLATFORM_MACOS_USE_CHAR16 0


namespace Rift
{
	/**
	 * Mac specific types
	 **/
	struct MacPlatformTypes : public GenericPlatformTypes
	{
		using DWORD        = unsigned int;
		using sizet        = std::size_t;
		using TYPE_OF_NULL = decltype(nullptr);
#if PLATFORM_MACOS_USE_CHAR16
		using WideChar = char16_t;
		using TChar    = WideChar;
#else
		using Char16 = char16_t;
#endif
	};

	using PlatformTypes = MacPlatformTypes;
}    // namespace Rift

#if BUILD_DEBUG
#	define FORCEINLINE inline /* Don't force code to be inline */
#else
#	define FORCEINLINE inline __attribute__((always_inline)) /* Force code to be inline */
#endif
#define NOINLINE __attribute__((noinline))

#define PLATFORM_BREAK() __asm__("int $3")

#if PLATFORM_MACOS_USE_CHAR16
#	undef PLATFORM_TCHAR_IS_CHAR16
#	define PLATFORM_TCHAR_IS_CHAR16 1
#endif
