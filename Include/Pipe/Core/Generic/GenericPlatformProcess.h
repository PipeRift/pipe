// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Export.h"


namespace p
{
	struct PIPE_API GenericPlatformProcess
	{
		static StringView GetExecutableFile() = delete;
		static StringView GetExecutablePath() = delete;
		static StringView GetBasePath()       = delete;

		static StringView GetUserPath();
		static StringView GetUserTempPath();
		static StringView GetUserHomePath();
		static StringView GetUserSettingsPath();
		static StringView GetAppSettingsPath();

		static String GetCurrentWorkingPath()              = delete;
		static void SetCurrentWorkingPath(StringView path) = delete;

		/** Content saved to compiler or project directories should be rerouted to user directories
		 * instead **/
		static bool ShouldSaveToUserDir()
		{
			return false;
		}
		static void ShowFolder(StringView path);
	};
}    // namespace p
