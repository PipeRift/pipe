// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Generic/GenericPlatformMisc.h"


namespace Pipe::Core
{
	/**
	 * Windows specific types
	 **/
	struct CORE_API WindowsPlatformMisc : public GenericPlatformMisc
	{
		static u32 GetMaxPathLength();
	};

	using PlatformMisc = WindowsPlatformMisc;
}    // namespace Pipe::Core

namespace Pipe
{
	using namespace Pipe::Core;
}
