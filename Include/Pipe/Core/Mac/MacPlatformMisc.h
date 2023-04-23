// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Generic/GenericPlatformMisc.h"


namespace p::core
{
	struct MacPlatformMisc : public GenericPlatformMisc
	{
		static constexpr u32 GetMaxPathLength()
		{
			return 1024;
		}
	};

	using PlatformMisc = MacPlatformMisc;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
