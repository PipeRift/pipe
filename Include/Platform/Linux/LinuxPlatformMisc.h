// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Generic/GenericPlatformMisc.h"


namespace Rift
{
	struct LinuxPlatformTypes : public GenericPlatformTypes
	{
		static constexpr u32 GetMaxPathLength()
		{
			return PATH_MAX;
		}
	};

	using PlatformMisc = LinuxPlatformMisc;
}    // namespace Rift
