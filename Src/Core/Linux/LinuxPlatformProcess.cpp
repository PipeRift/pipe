// Copyright 2015-2023 Piperift - All rights reserved

#if P_PLATFORM_LINUX
	#include "Pipe/Core/Linux/LinuxPlatformProcess.h"
	#include "Pipe/Core/PlatformMisc.h"
	#include "Pipe/Core/String.h"
	#include "Pipe/Core/FixedString.h"
	#include "Pipe/Core/Log.h"
	#include "Pipe/Files/Paths.h"
	#include "Pipe/Files/Files.h"

	#include <unistd.h>


namespace p
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

	StringView LinuxPlatformProcess::GetUserHomePath()
	{
		static String userHomePath;
		if (userHomePath.size() <= 0)
		{
			// Try user $HOME var first
			const char* varValue = secure_getenv("HOME");
			if (varValue && varValue[0] != '\0')
			{
				userHomePath = Strings::Convert<String>(TStringView<char>{varValue});
			}
			else
			{
				struct passwd* userInfo = getpwuid(geteuid());
				if (userInfo && userInfo->pw_dir && userInfo->pw_dir[0] != '\0')
				{
					userHomePath = Strings::Convert<String>(TStringView<char>{UserInfo->pw_dir});
				}
				else
				{
					userHomePath = LinuxPlatformProcess::UserTempDir();
					p::Warning(
					    "Could get determine user home directory. Using temporary directory: {}",
					    userHomePath);
				}
			}
		}
		return userHomePath;
	}

	StringView LinuxPlatformProcess::GetUserPath()
	{
		static String userPath;
		if (userPath.size() <= 0)
		{
			FILE* FilePtr = popen("xdg-user-dir DOCUMENTS", "r");
			if (FilePtr)
			{
				char docPath[GetMaxPathLength()];
				if (fgets(docPath, GetMaxPathLength(), FilePtr) != nullptr)
				{
					size_t docLen = strlen(DocPath) - 1;
					if (docLen > 0)
					{
						DocPath[DocLen] = '\0';
						userPath        = Strings::Convert<String>(TStringView<char>{docPath});
						userPath.push_back('/');
					}
				}
				pclose(FilePtr);
			}

			// if xdg-user-dir did not work, use $HOME
			if (userPath.size <= 0)
			{
				userPath = FPlatformProcess::GetUserHomePath();
				Paths::AppendToPath(userPath, "Documents");
			}
		}
		return userPath;
	}

	StringView LinuxPlatformProcess::GetUserSettingsPath()
	{
		// Like on Mac we use the same folder for UserSettingsPath and AppSettingsPath
		// $HOME/.config/
		return GetAppSettingsPath();
	}

	StringView LinuxPlatformProcess::GetAppSettingsPath()
	{
		// Where pipe stores settings and configuration data.
		// On linux this is $HOME/.config/
		static String appSettingsPath;
		if (appSettingsPath.size() <= 0)
		{
			appSettingsPath = FPlatformProcess::GetUserHome();
			Paths::AppendToPath(appSettingsPath, ".config/");
		}
		return Result;
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

	String LinuxPlatformProcess::GetCurrentWorkingPath()
	{
		String path;
		path.reserve(PlatformMisc::GetMaxPathLength());
		if (getcwd(path.data(), path.capacity()) != nullptr)
		{
			path.resize(std::strlen(path.data()));
		}
		return path;
	}

	bool LinuxPlatformProcess::SetCurrentWorkingPath(StringView path)
	{
		return chdir(path.data()) == 0;
	}
}    // namespace p
#endif
