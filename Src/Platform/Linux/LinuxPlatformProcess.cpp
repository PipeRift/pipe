// Copyright 2015-2022 Piperift - All rights reserved

#if PLATFORM_LINUX
#	include "Platform/Linux/LinuxPlatformProcess.h"
#	include "Platform/PlatformMisc.h"
#	include "Core/String.h"
#	include "Core/FixedString.h"
#	include "Files/Paths.h"
#	include "Files/Files.h"

#	include <unistd.h>


namespace Pipe
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
		return Paths::GetParent(GetExecutableFile());
	}

	StringView LinuxPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}

	void LinuxPlatformProcess::ShowFolder(StringView path)
	{
		if (!Files::Exists(path))
		{
			return;
		}

		if (!Files::IsFolder(path))
		{
			path = Paths::GetParent(path);
		}
		String fullPath{path};

		// launch file manager
		pid_t pid = fork();
		if (pid == 0)
		{
			exit(execl("/usr/bin/xdg-open", "xdg-open", fullPath.data(), (char*)0));
		}
	}
}    // namespace Pipe
#endif
