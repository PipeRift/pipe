// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include <date/date.h>
#include <date/tz.h>

#include <chrono>


namespace Pipe
{
	struct CORE_API PlatformTime
	{
		static u32 Cycles()
		{
			return u32(Cycles64());
		}
		static u64 Cycles64();
	};
}    // namespace Pipe
