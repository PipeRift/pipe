// Copyright 2015-2021 Piperift - All rights reserved

#include "Platform/Generic/GenericPlatformMisc.h"

#include "Misc/DateTime.h"
#include "Misc/Guid.h"
#include "Misc/Timespan.h"
#include "Platform/PlatformTime.h"


namespace Rift
{
	void GenericPlatformMisc::CreateGuid(Guid& guid)
	{
		static u16 incrementCounter = 0;
		static DateTime initialDateTime;

		DateTime estimatedCurrentDateTime = DateTime::Now();
		if (incrementCounter == 0)
		{
			initialDateTime = estimatedCurrentDateTime;
		}

		// Add sequential bits to ensure sequentially generated guids
		// are unique even if Cycles is wrong
		u32 sequentialBits = static_cast<u32>(incrementCounter);
		++incrementCounter;

		// Add randomness to improve uniqueness across machines
		u32 randBits = Math::Rand() & 0xFFFF;

		guid = Guid(randBits | (sequentialBits << 16), estimatedCurrentDateTime.GetTicks() >> 32,
		    estimatedCurrentDateTime.GetTicks() & 0xffffffff, PlatformTime::Cycles());
	}
}    // namespace Rift
