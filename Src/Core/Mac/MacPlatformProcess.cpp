// Copyright 2015-2022 Piperift - All rights reserved

#if PLATFORM_MACOS
#	include "Pipe/Core/Mac/MacPlatformProcess.h"
#	include "Pipe/Core/PlatformMisc.h"
#	include "Pipe/Core/String.h"
#	include "Pipe/Core/FixedString.h"
#	include "Pipe/Files/Paths.h"
#	include "Pipe/Files/Files.h"

#	include <mach-o/dyld.h>
#	include <mach/thread_act.h>
#	include <mach/thread_policy.h>
#	include <libproc.h>


namespace p::core
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

			filePath = Strings::Convert<String>(
			    TStringView<char>{rawPath.data(), Strings::Length(rawPath.data())});
		}
		return filePath;
	}
	StringView MacPlatformProcess::GetExecutablePath()
	{
		return GetParentPath(GetExecutableFile());
	}

	StringView MacPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}

	void MacPlatformProcess::ShowFolder(StringView path)
	{
		if (!files::Exists(path))
		{
			return;
		}

		NotImplemented;
		return;
	}
}    // namespace p::core
#endif