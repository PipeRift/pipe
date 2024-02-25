// Copyright 2015-2023 Piperift - All rights reserved

#if P_PLATFORM_LINUX
	#include "Pipe/Files/PlatformPaths.h"

	#include "Pipe/Core/PlatformMisc.h"
	#include "Pipe/Core/String.h"
	#include "Pipe/Core/FixedString.h"
	#include "Pipe/Core/Log.h"
	#include "Pipe/Files/Paths.h"
	#include "Pipe/Files/Files.h"

	#include <unistd.h>
	#include <pwd.h>

namespace p
{
	StringView LinuxPlatformPaths::GetExecutableFile()
	{
		static String path;
		if (path.empty())
		{
			TFixedString<GetMaxPathLength(), char> rawPath{};
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
	StringView LinuxPlatformPaths::GetExecutablePath()
	{
		return GetParentPath(GetExecutableFile());
	}

	StringView LinuxPlatformPaths::GetBasePath()
	{
		return GetExecutablePath();
	}

	StringView LinuxPlatformPaths::GetUserPath()
	{
		static String userPath;
		if (userPath.empty())
		{
			FILE* FilePtr = popen("xdg-user-dir DOCUMENTS", "r");
			if (FilePtr)
			{
				char docPath[GetMaxPathLength()];
				if (fgets(docPath, GetMaxPathLength(), FilePtr) != nullptr)
				{
					size_t docLen = strlen(docPath) - 1;
					if (docLen > 0)
					{
						docPath[docLen] = '\0';
						userPath        = Strings::Convert<String>(TStringView<char>{docPath});
						userPath.push_back('/');
					}
				}
				pclose(FilePtr);
			}

			// if xdg-user-dir did not work, use $HOME
			if (userPath.empty())
			{
				userPath = PlatformPaths::GetUserHomePath();
				AppendToPath(userPath, "Documents");
			}
		}
		return userPath;
	}

	StringView LinuxPlatformPaths::GetUserTempPath()
	{
		// Use $TMPDIR if its set otherwise fallback to /var/tmp as Windows defaults to %TEMP% which
		// does not get cleared on reboot.
		static String userTempPath;
		if (userTempPath.empty())
		{
			const char* dirValue = secure_getenv("TMPDIR");
			if (dirValue)
			{
				userTempPath = Strings::Convert<String>(TStringView<char>{dirValue});
			}
			else
			{
				userTempPath = "/var/tmp";
			}
		}
		return userTempPath;
	}

	StringView LinuxPlatformPaths::GetUserHomePath()
	{
		static String userHomePath;
		if (userHomePath.empty())
		{
			// Try user $HOME var first
			const char* dirValue = secure_getenv("HOME");
			if (dirValue && dirValue[0] != '\0')
			{
				userHomePath = Strings::Convert<String>(TStringView<char>{dirValue});
			}
			else
			{
				struct passwd* userInfo = getpwuid(geteuid());
				if (userInfo && userInfo->pw_dir && userInfo->pw_dir[0] != '\0')
				{
					userHomePath = Strings::Convert<String>(TStringView<char>{userInfo->pw_dir});
				}
				else
				{
					userHomePath = GetUserTempPath();
					p::Warning(
					    "Could get determine user home directory. Using temporary directory: {}",
					    userHomePath);
				}
			}
		}
		return userHomePath;
	}

	StringView LinuxPlatformPaths::GetUserSettingsPath()
	{
		// Like on Mac we use the same folder for UserSettingsPath and AppSettingsPath
		// $HOME/.config/
		return GetAppSettingsPath();
	}

	StringView LinuxPlatformPaths::GetAppSettingsPath()
	{
		// Where pipe stores settings and configuration data.
		// On linux this is $HOME/.config/
		static String appSettingsPath;
		if (appSettingsPath.empty())
		{
			appSettingsPath = PlatformPaths::GetUserHomePath();
			AppendToPath(appSettingsPath, ".config/");
		}
		return appSettingsPath;
	}

	void LinuxPlatformPaths::ShowFolder(StringView path)
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

	StringView LinuxPlatformPaths::GetCurrentPath()
	{
		static String path;
		path.reserve(GetMaxPathLength());
		if (getcwd(path.data(), path.capacity()) != nullptr)
		{
			path.resize(std::strlen(path.data()));
		}
		return path;
	}

	bool LinuxPlatformPaths::SetCurrentPath(StringView path)
	{
		return chdir(path.data()) == 0;
	}
}    // namespace p
#endif
