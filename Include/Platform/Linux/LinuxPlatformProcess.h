// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Export.h"
#include "Platform/Generic/GenericPlatformProcess.h"


namespace Rift
{
	struct CORE_API LinuxPlatformProcess : public GenericPlatformProcess
	{
		static StringView GetModuleFilePath();
		static StringView GetBasePath();
		// static StringView GetUserDir();
		// static StringView GetUserTempDir();
		// static StringView GetUserSettingsDir();
		// static StringView GetApplicationSettingsDir();
	};

	using PlatformProcess = LinuxPlatformProcess;
}    // namespace Rift
