// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace p::core
{
	struct Guid;

	/**
	 * Generic types for almost all compilers and platforms
	 */
	struct CORE_API GenericPlatformMisc
	{
		static void CreateGuid(core::Guid& guid);

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
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
