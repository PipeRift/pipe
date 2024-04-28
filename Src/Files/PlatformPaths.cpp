// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Files/PlatformPaths.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Core/String.h"
#include "Pipe/Files/Files.h"
#include "Pipe/Files/Paths.h"


#if P_PLATFORM_WINDOWS
	#include <Windows.h>
	#include <combaseapi.h>
	#include <shlobj.h> /* SHGetKnownFolderPath(), FOLDERID_... */
#elif P_PLATFORM_LINUX
	#include <unistd.h>
	#include <pwd.h>
#elif P_PLATFORM_MACOS
	#include <mach-o/dyld.h>
	#include <mach/thread_act.h>
	#include <mach/thread_policy.h>
	#include <libproc.h>
	#include <fcntl.h>
	#include <unistd.h>
#endif


namespace p
{

	StringView GenericPlatformPaths::GetUserPath()
	{
		// default to the base directory
		return PlatformPaths::GetBasePath();
	}

	StringView GenericPlatformPaths::GetUserTempPath()
	{
		// default to the base directory
		return PlatformPaths::GetBasePath();
	}

	StringView GenericPlatformPaths::GetUserSettingsPath()
	{
		// default to the base directory
		return PlatformPaths::GetBasePath();
	}

	StringView GenericPlatformPaths::GetAppSettingsPath()
	{
		// default to the base directory
		return PlatformPaths::GetBasePath();
	}

	void GenericPlatformPaths::ShowFolder(StringView path)
	{
		Error("ShowFolder not implemented on this platform");
	}


#if P_PLATFORM_WINDOWS

	bool AreLongPathsEnabled()
	{
		if (HMODULE Handle = GetModuleHandle(TEXT("ntdll.dll")))
		{
			using EnabledFunction = BOOLEAN(NTAPI*)();
			auto RtlAreLongPathsEnabled =
			    (EnabledFunction)(void*)GetProcAddress(Handle, "RtlAreLongPathsEnabled");

			return RtlAreLongPathsEnabled != nullptr && RtlAreLongPathsEnabled();
		}
		return false;
	}

	template<typename StringType, typename TStringGetterFunc>
	static void GetStringFromWindowsAPI(
	    StringType& result, TStringGetterFunc stringGetter, u32 initialSize = 128)
	{
		if (initialSize <= 0)
		{
			initialSize = 128;    // MAX_PATH. Not worth including Windows.h
		}

		for (u32 length = initialSize;;)
		{
			result.resize(length);
			length = stringGetter(result.data(), result.size());
			if (length == 0)
			{
				result.clear();
				break;
			}
			else if (length <= u32(result.size()))
			{
				result.resize(length);
				result.shrink_to_fit();
				break;
			}
		}
	}

	u32 WindowsPlatformPaths::GetMaxPathLength()
	{
		static const u32 maxPath = AreLongPathsEnabled() ? 32767 : MAX_PATH;
		return maxPath;
	}

	StringView WindowsPlatformPaths::GetExecutableFile()
	{
		static String executablePath;
		if (executablePath.empty())
		{
			GetStringFromWindowsAPI(executablePath, [](char* buffer, sizet size) {
				return GetModuleFileNameA(nullptr, buffer, u32(size));
			});
		}
		return executablePath;
	}

	StringView WindowsPlatformPaths::GetExecutablePath()
	{
		return GetParentPath(GetExecutableFile());
	}

	StringView WindowsPlatformPaths::GetBasePath()
	{
		return GetExecutablePath();
	}

	StringView WindowsPlatformPaths::GetUserPath()
	{
		static String userPath;
		if (userPath.empty())
		{
			// get the local AppData directory
			WideChar* path = nullptr;
			HRESULT Ret    = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &path);
			if (SUCCEEDED(Ret))
			{
				// make the base user dir path
				userPath = Strings::Convert<String>(TStringView<WideChar>{path});
				std::replace(userPath.begin(), userPath.end(), '\\', '/');
				userPath.push_back('/');
				CoTaskMemFree(path);
			}
		}
		return userPath;
	}

	StringView WindowsPlatformPaths::GetUserTempPath()
	{
		static String userTempPath;
		if (userTempPath.empty())
		{
			char tempPath[MAX_PATH];
			ZeroMemory(tempPath, sizeof(char) * MAX_PATH);

			::GetTempPath(MAX_PATH, tempPath);

			// Always expand the temp path in case windows returns short directory names.
			char fullTempPath[MAX_PATH];
			ZeroMemory(fullTempPath, sizeof(char) * MAX_PATH);
			::GetLongPathName(tempPath, fullTempPath, MAX_PATH);

			userTempPath = Strings::Convert<String>(TStringView<char>{fullTempPath});
			std::replace(userTempPath.begin(), userTempPath.end(), '\\', '/');
		}
		return userTempPath;
	}

	StringView WindowsPlatformPaths::GetUserSettingsPath()
	{
		static String userSettingsPath;
		if (userSettingsPath.empty())
		{
			// get the local AppData directory
			WideChar* path = nullptr;
			HRESULT Ret    = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &path);
			if (SUCCEEDED(Ret))
			{
				// make the base user dir path
				userSettingsPath = Strings::Convert<String>(TStringView<WideChar>{path});
				std::replace(userSettingsPath.begin(), userSettingsPath.end(), '\\', '/');
				userSettingsPath.push_back('/');
				CoTaskMemFree(path);
			}
		}
		return userSettingsPath;
	}

	StringView WindowsPlatformPaths::GetAppSettingsPath()
	{
		static String appSettingsPath;
		if (!appSettingsPath.size())
		{
			// get the local AppData directory
			WideChar* path = nullptr;
			HRESULT Ret    = SHGetKnownFolderPath(FOLDERID_ProgramData, 0, NULL, &path);
			if (SUCCEEDED(Ret))
			{
				// make the base user dir path
				appSettingsPath = Strings::Convert<String>(TStringView<WideChar>{path});
				std::replace(appSettingsPath.begin(), appSettingsPath.end(), '\\', '/');
				CoTaskMemFree(path);
			}
		}
		return appSettingsPath;
	}

	StringView WindowsPlatformPaths::GetCurrentPath()
	{
		static String path;
		GetStringFromWindowsAPI(path, [](char* buffer, sizet size) {
			return ::GetCurrentDirectoryA(size, buffer);
		});
		return path;
	}

	bool WindowsPlatformPaths::SetCurrentPath(StringView path)
	{
	#if PLATFORM_TCHAR_IS_WCHAR
		return ::SetCurrentDirectoryW(path.data());
	#else
		return ::SetCurrentDirectoryA(path.data());
	#endif
	}


	void WindowsPlatformPaths::ShowFolder(StringView path)
	{
		if (!Exists(path))
		{
			return;
		}

		if (IsFolder(path))
		{
			const String fullPath{path};
			::ShellExecuteA(nullptr, "explore", fullPath.data(), nullptr, nullptr, SW_SHOWNORMAL);
		}
		else if (IsFile(path))
		{
			String parameters = Strings::Format("/select,{}", path);
			::ShellExecuteA(
			    nullptr, "open", "explorer.exe", parameters.data(), nullptr, SW_SHOWNORMAL);
		}
	}

#elif P_PLATFORM_LINUX

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
		if (!Exists(path))
		{
			return;
		}

		if (!IsFolder(path))
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

#elif P_PLATFORM_MACOS

	StringView MacPlatformPaths::GetExecutableFile()
	{
		static String filePath;
		if (filePath.empty())
		{
			TFixedString<GetMaxPathLength(), char> rawPath{};
			u32 size = u32(rawPath.size());
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

	StringView MacPlatformPaths::GetUserTempPath()
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

	StringView MacPlatformPaths::GetUserHomePath()
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

	StringView MacPlatformPaths::GetUserPath()
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

	StringView MacPlatformPaths::GetUserSettingsPath()
	{
		return GetAppSettingsPath();
	}

	StringView MacPlatformPaths::GetAppSettingsPath()
	{
		static String appSettingsPath;
		if (appSettingsPath.empty())
		{
			NSString* ApplicationSupportFolder = [NSSearchPathForDirectoriesInDomains(
			    NSApplicationSupportDirectory, NSUserDomainMask, YES) objectAtIndex:0];
			userPath                           = String{[ApplicationSupportFolder UTF8String]};
			userPath.push_back('/');
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
		if (!Exists(path))
		{
			return;
		}

		NotImplemented;
		return;
	}

#endif
}    // namespace p
