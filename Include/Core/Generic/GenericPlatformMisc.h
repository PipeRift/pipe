// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Pipe::Core
{
	struct Guid;

	/**
	 * Generic types for almost all compilers and platforms
	 */
	struct CORE_API GenericPlatformMisc
	{
		static void CreateGuid(Core::Guid& guid);

		static constexpr u32 GetMaxPathLength()
		{
			return 128;
		}

		static u64 GetCycles64();

		static u32 GetCycles()
		{
			return u32(GetCycles64());
		}
	};
}    // namespace Pipe::Core

namespace Pipe
{
	using namespace Pipe::Core;
}
