// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Generic/GenericPlatformMisc.h"

#include <linux/limits.h>


namespace p::core
{
	struct LinuxPlatformMisc : public GenericPlatformMisc
	{
		static constexpr u32 GetMaxPathLength()
		{
			return PATH_MAX;
		}
	};

	using PlatformMisc = LinuxPlatformMisc;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}