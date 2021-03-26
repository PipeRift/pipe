// Copyright 2015-2021 Piperift - All rights reserved

#if PLATFORM_MACOS
#	include "Platform/Mac/MacPlatformProcess.h"
#	include "Platform/PlatformMisc.h"
#	include "Strings/String.h"
#	include "Strings/FixedString.h"
#	include "Files/Paths.h"

#	include <mach-o/dyld.h>


namespace Rift
{
	StringView MacPlatformProcess::GetExecutableFile()
	{
		static String filePath;
		if (filePath.empty())
		{
			TFixedString<char, PlatformMisc::GetMaxPathLength()> rawPath{};
			if (_NSGetExecutablePath(rawPath.data(), rawPath.size()) != 0)
			{
				// Failed to retrive a path
				return {};
			}

			filePath = CString::Convert(
			    TStringView<char>{rawPath.data(), CString::Length(rawPath.data())});
		}
		return filePath;
	}
	StringView MacPlatformProcess::GetExecutablePath()
	{
		return Paths::GetParent(GetExecutableFile());
	}

	StringView MacPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}
}    // namespace Rift
#endif
