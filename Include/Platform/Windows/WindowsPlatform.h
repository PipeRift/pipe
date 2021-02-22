// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/GenericPlatform.h"

#include <cstddef>


namespace Rift
{
	/**
	 * Windows specific types
	 **/
	struct CORE_API WindowsPlatformTypes : public GenericPlatformTypes
	{
		using sizet = std::size_t;
	};

	using PlatformTypes = WindowsPlatformTypes;
}    // namespace Rift

#define FORCEINLINE __forceinline     /* Force code to be inline */
#define NOINLINE __declspec(noinline) /* Force code to not be inlined */
