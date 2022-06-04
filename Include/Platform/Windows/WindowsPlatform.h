// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Platform/Generic/GenericPlatform.h"

#include <cstddef>


namespace Pipe
{
	/**
	 * Windows specific types
	 **/
	struct CORE_API WindowsPlatformTypes : public GenericPlatformTypes
	{
		using sizet = std::size_t;
	};

	using PlatformTypes = WindowsPlatformTypes;
}    // namespace Pipe

#define FORCEINLINE __forceinline     /* Force code to be inline */
#define NOINLINE __declspec(noinline) /* Force code to not be inlined */

#define PLATFORM_BREAK() (__nop(), __debugbreak())
