// Copyright 2015-2021 Piperift - All rights reserved

#if PLATFORM_MAC
#	include "Platform/Linux/LinuxPlatformProcess.h"
#	include "Platform/PlatformMisc.h"
#	include "Strings/String.h"


namespace Rift
{
	StringView MacPlatformProcess::GetExecutableFile()
	{
		return {};
	}
	StringView MacPlatformProcess::GetExecutablePath()
	{
		return Paths::GetParent(GetExecutableFile());
	}

	StringView MacPlatformProcess::GetBasePath()
	{
		return {};
	}
}    // namespace Rift
#endif
