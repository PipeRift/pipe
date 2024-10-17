// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/GenericPlatformMisc.h"
#include "Pipe/Core/StringView.h"


namespace p
{
	/**
	 * Windows specific types
	 **/
	struct PIPE_API WindowsPlatformMisc : public GenericPlatformMisc
	{
		static const char* GetSystemErrorMessage(char* buffer, i32 size, i32 error = 0);
	};

	using PlatformMisc = WindowsPlatformMisc;
}    // namespace p
