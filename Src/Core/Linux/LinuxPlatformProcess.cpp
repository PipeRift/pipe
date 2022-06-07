// Copyright 2015-2022 Piperift - All rights reserved

#if PLATFORM_LINUX
#	include "Pipe/Core/Linux/LinuxPlatformProcess.h"
#	include "Pipe/Core/PlatformMisc.h"
#	include "Pipe/Core/String.h"
#	include "Pipe/Core/FixedString.h"
#	include "Pipe/Files/Paths.h"
#	include "Pipe/Files/Files.h"

#	include <unistd.h>


namespace p::core
{
	StringView LinuxPlatformProcess::GetExecutableFile()
	{
		static String path;
		if (path.empty())
		{
			TFixedString<PlatformMisc::GetMaxPathLength(), char> rawPath{};
			if (readlink("/proc/self/exe", rawPath.data(), rawPath.size() - 1) == -1)
			{
				// readlink() failed. Unreachable
				return {};
			}

			path = Strings::Convert<String>(
			    TStringView<char>{rawPath.data(), Strings::Length(rawPath.data())});
		}
		return path;
	}
	StringView LinuxPlatformProcess::GetExecutablePath()
	{
		return GetParentPath(GetExecutableFile());
	}

	StringView LinuxPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}

	void LinuxPlatformProcess::ShowFolder(StringView path)
	{
		if (!files::Exists(path))
		{
			return;
		}

		if (!files::IsFolder(path))
		{
			path = GetParentPath(path);
		}
		String fullPath{path};

		// launch file manager
		pid_t pid = fork();
		if (pid == 0)
		{
			exit(execl("/usr/bin/xdg-open", "xdg-open", fullPath.data(), (char*)0));
		}
	}
}    // namespace p::core
#endif
