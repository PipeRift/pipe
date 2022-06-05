// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Generic/GenericPlatform.h"

#include <cstddef>


namespace p::core
{
	/**
	 * Windows specific types
	 **/
	struct CORE_API WindowsPlatformTypes : public GenericPlatformTypes
	{
		using sizet = std::size_t;
	};

	using PlatformTypes = WindowsPlatformTypes;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}

#define FORCEINLINE __forceinline     /* Force code to be inline */
#define NOINLINE __declspec(noinline) /* Force code to not be inlined */

#define PLATFORM_BREAK() (__nop(), __debugbreak())
