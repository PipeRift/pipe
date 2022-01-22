// Copyright 2015-2022 Piperift - All rights reserved

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
			TFixedString<PlatformMisc::GetMaxPathLength(), char> rawPath{};
			u32 size{rawPath.size()};
			if (_NSGetExecutablePath(rawPath.data(), &size) != 0)
			{
				// Failed to retrive a path
				return {};
			}

			filePath = Strings::Convert(
			    TStringView<char>{rawPath.data(), Strings::Length(rawPath.data())});
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
