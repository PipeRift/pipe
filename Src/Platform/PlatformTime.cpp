// Copyright 2015-2022 Piperift - All rights reserved

#include "Platform/PlatformTime.h"

#include "Misc/DateTime.h"


namespace Pipe
{
	u64 PlatformTime::Cycles64()
	{
		return Chrono::floor<Chrono::microseconds>(DateTime::Now().GetTime().time_since_epoch())
		    .count();
	}
}    // namespace Pipe
