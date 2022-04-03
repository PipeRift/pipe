// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Export.h"
#include "Platform/Generic/GenericPlatformProcess.h"


namespace Rift
{
	struct CORE_API LinuxPlatformProcess : public GenericPlatformProcess
	{
		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static void ShowFolder(StringView path);
	};

	using PlatformProcess = LinuxPlatformProcess;
}    // namespace Rift
