// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Generic/GenericPlatformProcess.h"
#include "Pipe/Export.h"


namespace p::core
{
	struct PIPE_API MacPlatformProcess : public GenericPlatformProcess
	{
		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static StringView GetUserHomePath();
		static StringView GetUserPath();
		static StringView GetUserSettingsPath();
		static StringView GetAppSettingsPath();

		static String GetCurrentWorkingPath();
		static bool SetCurrentWorkingPath(StringView path);

		static void ShowFolder(StringView path);
	};
	using PlatformProcess = MacPlatformProcess;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
