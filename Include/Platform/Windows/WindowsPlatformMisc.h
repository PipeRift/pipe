// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Platform/Generic/GenericPlatformMisc.h"


namespace Pipe
{
	/**
	 * Windows specific types
	 **/
	struct CORE_API WindowsPlatformMisc : public GenericPlatformMisc
	{
		static u32 GetMaxPathLength();
	};

	using PlatformMisc = WindowsPlatformMisc;
}    // namespace Pipe
