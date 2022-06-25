// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Generic/GenericPlatformMisc.h"
#include "Pipe/Core/StringView.h"


namespace p::core
{
	/**
	 * Windows specific types
	 **/
	struct PIPE_API WindowsPlatformMisc : public GenericPlatformMisc
	{
		static u32 GetMaxPathLength();

		static const TChar* GetSystemErrorMessage(TChar* buffer, i32 size, i32 error = 0);
	};

	using PlatformMisc = WindowsPlatformMisc;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
