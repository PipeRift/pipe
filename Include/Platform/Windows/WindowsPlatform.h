// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/GenericPlatform.h"


namespace Rift
{
	/**
	 * Windows specific types
	 **/
	struct CORE_API WindowsPlatformTypes : public GenericPlatformTypes
	{
#ifdef _WIN64
		using sizet = __int64;
		using ssizet = __int64;
#else
		using sizet = unsigned long;
		using ssizet = long;
#endif
	};

	using PlatformTypes = WindowsPlatformTypes;
}	 // namespace Rift

#define FORCEINLINE __forceinline	  /* Force code to be inline */
#define NOINLINE __declspec(noinline) /* Force code to not be inlined */
