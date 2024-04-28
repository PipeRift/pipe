// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Export.h"


#if P_PLATFORM_LINUX
	#include <linux/limits.h>
#endif


namespace p
{
	struct PIPE_API GenericPlatformPaths
	{
		static constexpr u32 GetMaxPathLength()
		{
			return 128;
		}

		static StringView GetExecutableFile() = delete;
		static StringView GetExecutablePath() = delete;
		static StringView GetBasePath()       = delete;

		static StringView GetUserPath();
		static StringView GetUserTempPath();
		static StringView GetUserHomePath();
		static StringView GetUserSettingsPath();
		static StringView GetAppSettingsPath();

		static StringView GetCurrentPath()          = delete;
		static void SetCurrentPath(StringView path) = delete;

		/** Content saved to compiler or project directories should be rerouted to user directories
		 * instead **/
		static bool ShouldSaveToUserDir()
		{
			return false;
		}
		static void ShowFolder(StringView path);
	};


#if P_PLATFORM_WINDOWS
	struct PIPE_API WindowsPlatformPaths : public GenericPlatformPaths
	{
		static u32 GetMaxPathLength();

		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static StringView GetUserPath();
		static StringView GetUserTempPath();
		static StringView GetUserSettingsPath();
		static StringView GetAppSettingsPath();

		static StringView GetCurrentPath();
		static bool SetCurrentPath(StringView path);

		static void ShowFolder(StringView path);
	};
	using PlatformPaths = WindowsPlatformPaths;

#elif P_PLATFORM_LINUX
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

#elif P_PLATFORM_MACOS
	struct PIPE_API MacPlatformPaths : public GenericPlatformPaths
	{
		static constexpr u32 GetMaxPathLength()
		{
			return 1024;
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
	using PlatformPaths = MacPlatformPaths;
#else
	#error Unknown platform
#endif
}    // namespace p
