// Copyright 2015-2021 Piperift - All rights reserved

#if PLATFORM_LINUX
#	include "Platform/Linux/LinuxPlatformProcess.h"
#	include "Platform/PlatformMisc.h"
#	include "Strings/String.h"
#	include "Strings/FixedString.h"
#	include "Files/Paths.h"

#	include <unistd.h>

namespace Rift
{
	StringView LinuxPlatformProcess::GetExecutableFile()
	{
		static String filePath;
		if (filePath.empty())
		{
			TFixedString<char, PlatformMisc::GetMaxPathLength()> rawPath{};
			if (readlink("/proc/self/exe", rawPath.data(), rawPath.size() - 1) == -1)
			{
				// readlink() failed. Unreachable
				return {};
			}

			filePath = CString::Convert(
			    TStringView<char>{rawPath.data(), CString::Length(rawPath.data())});
		}
		return filePath;
	}
	StringView LinuxPlatformProcess::GetExecutablePath()
	{
		return Paths::GetParent(GetExecutableFile());
	}

	StringView LinuxPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}
}    // namespace Rift
#endif
