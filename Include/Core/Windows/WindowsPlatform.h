// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Generic/GenericPlatform.h"

#include <cstddef>


namespace Pipe::Core
{
	/**
	 * Windows specific types
	 **/
	struct CORE_API WindowsPlatformTypes : public GenericPlatformTypes
	{
		using sizet = std::size_t;
	};

	using PlatformTypes = WindowsPlatformTypes;
}    // namespace Pipe::Core

namespace Pipe
{
	using namespace Pipe::Core;
}

#define FORCEINLINE __forceinline     /* Force code to be inline */
#define NOINLINE __declspec(noinline) /* Force code to not be inlined */

#define PLATFORM_BREAK() (__nop(), __debugbreak())
