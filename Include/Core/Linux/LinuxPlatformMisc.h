// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Generic/GenericPlatformMisc.h"

#include <linux/limits.h>


namespace Pipe::Core
{
	struct LinuxPlatformMisc : public GenericPlatformMisc
	{
		static constexpr u32 GetMaxPathLength()
		{
			return PATH_MAX;
		}
	};

	using PlatformMisc = LinuxPlatformMisc;
}    // namespace Pipe::Core

namespace Pipe
{
	using namespace Pipe::Core;
}
