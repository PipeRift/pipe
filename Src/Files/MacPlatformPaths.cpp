// Copyright 2015-2023 Piperift - All rights reserved

#if P_PLATFORM_MACOS
	#include "Pipe/Files/MacPlatformPaths.h"

	#include "Pipe/Core/PlatformMisc.h"
	#include "Pipe/Core/String.h"
	#include "Pipe/Core/FixedString.h"
	#include "Pipe/Files/Paths.h"
	#include "Pipe/Files/Files.h"

	#include <mach-o/dyld.h>
	#include <mach/thread_act.h>
	#include <mach/thread_policy.h>
	#include <libproc.h>
	#include <fcntl.h>
	#include <unistd.h>


namespace p
{
	StringView MacPlatformPaths::GetExecutableFile()
	{
		static String filePath;
		if (filePath.empty())
		{
			TFixedString<GetMaxPathLength(), char> rawPath{};
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
	StringView MacPlatformPaths::GetExecutablePath()
	{
		return GetParentPath(GetExecutableFile());
	}

	StringView MacPlatformPaths::GetBasePath()
	{
		return GetExecutablePath();
	}

	const TCHAR* MacPlatformPaths::GetUserTempPath()
	{
		static String userTempDir;
		if (userTempDir.empty())
		{
			userTempDir = NSTemporaryDirectory();
		}
		return userTempDir;
	}

	StringView MacPlatformPaths::GetUserHomePath()
	{
		static String userHomePath;
		if (userHomePath.empty())
		{
			@autoreleasepool
			{
				userHomePath = String{[NSHomeDirectory() UTF8String]};
			}
		}
		return userHomePath;
	}

	StringView MacPlatformPaths::GetUserPath()
	{
		static String userPath;
		if (userPath.empty())
		{
			@autoreleasepool
			{
				NSString* DocumentsFolder = [NSSearchPathForDirectoriesInDomains(
				    NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0];
				userPath                  = String{[DocumentsFolder UTF8String]};
				userPath.push_back('/');
			}
		}
		return userPath;
	}

	StringView MacPlatformPaths::GetUserSettingsPath()
	{
		return GetAppSettingsPath();
	}

	StringView MacPlatformPaths::GetAppSettingsPath()
	{
		static String appSettingsPath;
		if (appSettingsPath.empty())
		{
			@autoreleasepool
			{
				NSString* ApplicationSupportFolder = [NSSearchPathForDirectoriesInDomains(
				    NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0];
				userPath                           = String{[ApplicationSupportFolder UTF8String]};
				userPath.push_back('/');
			}
		}
		return appSettingsPath;
	}


	StringView MacPlatformPaths::GetCurrentPath()
	{
		static String path;
		path.reserve(GetMaxPathLength());
		if (getcwd(path.data(), path.capacity()) != nullptr)
		{
			path.resize(std::strlen(path.data()));
		}
		return path;
	}

	bool MacPlatformPaths::SetCurrentPath(StringView path)
	{
		return chdir(path.data()) == 0;
	}

	void MacPlatformPaths::ShowFolder(StringView path)
	{
		if (!files::Exists(path))
		{
			return;
		}

		NotImplemented;
		return;
	}
}    // namespace p
#endif
