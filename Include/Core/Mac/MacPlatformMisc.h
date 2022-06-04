// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Generic/GenericPlatformMisc.h"


namespace Pipe::Core
{
	struct MacPlatformMisc : public GenericPlatformMisc
	{
		static constexpr u32 GetMaxPathLength()
		{
			return 1024;
		}
	};

	using PlatformMisc = MacPlatformMisc;
}    // namespace Pipe::Core

namespace Pipe
{
	using namespace Pipe::Core;
}
