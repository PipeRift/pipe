// Copyright 2015-2023 Piperift - All rights reserved

#if P_PLATFORM_MACOS
	#include "Pipe/Core/Mac/MacPlatformProcess.h"
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


	StringView MacPlatformProcess::GetUserHomePath()
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

	StringView MacPlatformProcess::GetUserPath()
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

	StringView MacPlatformProcess::GetUserSettingsPath()
	{
		return GetAppSettingsPath();
	}

	StringView MacPlatformProcess::GetAppSettingsPath()
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
}    // namespace p
#endif
