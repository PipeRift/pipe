// Copyright 2015-2021 Piperift - All rights reserved
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
	};

	using PlatformProcess = LinuxPlatformProcess;
}    // namespace Rift
