// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCore/Generic/GenericPlatformMisc.h"


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
