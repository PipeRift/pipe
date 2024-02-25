// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/GenericPlatformPaths.h"
#include "Pipe/Export.h"

#include <linux/limits.h>


namespace p
{
	struct PIPE_API LinuxPlatformPaths : public GenericPlatformPaths
	{
		static constexpr u32 GetMaxPathLength()
		{
			return PATH_MAX;
		}

		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static StringView GetUserPath();
		static StringView GetUserTempPath();
		static StringView GetUserHomePath();
		static StringView GetUserSettingsPath();
		static StringView GetAppSettingsPath();

		static StringView GetCurrentPath();
		static bool SetCurrentPath(StringView path);

		static void ShowFolder(StringView path);
	};
	using PlatformPaths = LinuxPlatformPaths;
}    // namespace p
