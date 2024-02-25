// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/StringView.h"
#include "Pipe/Export.h"


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
}    // namespace p
