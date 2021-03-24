// Copyright 2015-2021 Piperift - All rights reserved

#if PLATFORM_LINUX
#	include "Platform/Linux/LinuxPlatformProcess.h"
#	include "Strings/String.h"


namespace Rift
{
	StringView LinuxPlatformProcess::GetModuleFilePath()
	{
		return {};
	}

	StringView LinuxPlatformProcess::GetBasePath()
	{
		return GetModuleFilePath();
	}
}    // namespace Rift
#endif
