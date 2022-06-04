// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Pipe
{
	/**
	 * Generic types for almost all compilers and platforms
	 */
	struct CORE_API GenericPlatformMisc
	{
		static void CreateGuid(struct Guid& guid);

		static constexpr u32 GetMaxPathLength()
		{
			return 128;
		}
	};
}    // namespace Pipe
