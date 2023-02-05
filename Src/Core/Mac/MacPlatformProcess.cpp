// Copyright 2015-2023 Piperift - All rights reserved

#if P_PLATFORM_MACOS
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
#	include <fcntl.h>


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

	String MacPlatformProcess::GetCurrentWorkingPath()
	{
		String path;
		path.reserve(PlatformMisc::GetMaxPathLength());
		if (getcwd(path.data(), path.capacity()) != nullptr)
		{
			path.resize(std::strlen(path.data()));
		}
		return path;
	}

	bool MacPlatformProcess::SetCurrentWorkingPath(StringView path)
	{
		return chdir(path.data()) == 0;
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
