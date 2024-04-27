// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Core/GenericPlatformMisc.h"

#include "Pipe/Core/Guid.h"
#include "Pipe/Core/PlatformMisc.h"
#include "PipeTime.h"


namespace p
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
		u32 randBits = Rand() & 0xFFFF;

		guid = Guid(randBits | (sequentialBits << 16), estimatedCurrentDateTime.GetTicks() >> 32,
		    estimatedCurrentDateTime.GetTicks() & 0xffffffff, PlatformMisc::GetCycles());
	}

	u64 GenericPlatformMisc::GetCycles64()
	{
		return Chrono::floor<Chrono::microseconds>(DateTime::Now().GetTime().time_since_epoch())
		    .count();
	}
}    // namespace p
