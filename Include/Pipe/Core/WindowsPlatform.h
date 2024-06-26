// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/GenericPlatform.h"

#include <intrin.h>

#include <cstddef>


namespace p
{
	/**
	 * Windows specific types
	 **/
	struct PIPE_API WindowsPlatformTypes : public GenericPlatformTypes
	{
		using sizet = std::size_t;
	};

	using PlatformTypes = WindowsPlatformTypes;
}    // namespace p


#define P_FORCEINLINE __forceinline     /* Force code to be inline */
#define P_NOINLINE __declspec(noinline) /* Force code to not be inlined */

#define P_PLATFORM_BREAK() (__nop(), __debugbreak())

#if !defined(__clang__)
	#define DISABLE_OPTIMIZATION_ACTUAL __pragma(optimize("", off))
	#define ENABLE_OPTIMIZATION_ACTUAL __pragma(optimize("", on))
#elif defined(_MSC_VER)    // Clang only supports __pragma with -fms-extensions
	#define DISABLE_OPTIMIZATION_ACTUAL __pragma(clang optimize off)
	#define ENABLE_OPTIMIZATION_ACTUAL __pragma(clang optimize on)
#endif
