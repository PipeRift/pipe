// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Platform/Generic/GenericPlatformMisc.h"


namespace Pipe
{
	struct MacPlatformMisc : public GenericPlatformMisc
	{
		static constexpr u32 GetMaxPathLength()
		{
			return 1024;
		}
	};

	using PlatformMisc = MacPlatformMisc;
}    // namespace Pipe
