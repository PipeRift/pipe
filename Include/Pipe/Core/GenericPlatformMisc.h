// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Export.h"


namespace p
{
	struct Guid;

	/**
	 * Generic types for almost all compilers and platforms
	 */
	struct PIPE_API GenericPlatformMisc
	{
		static void CreateGuid(Guid& guid);

		static u64 GetCycles64();

		static u32 GetCycles()
		{
			return u32(GetCycles64());
		}
	};
}    // namespace p
