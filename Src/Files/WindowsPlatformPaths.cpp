// Copyright 2015-2023 Piperift - All rights reserved

#if P_PLATFORM_WINDOWS
	#include "Pipe/Files/PlatformPaths.h"

	#include "Pipe/Files/Paths.h"
	#include "Pipe/Files/Files.h"
	#include "Pipe/Core/Platform.h"
	#include "Pipe/Core/WindowsPlatformMisc.h"
	#include "Pipe/Core/String.h"
	#include "Pipe/Core/Log.h"

	#include <Windows.h>
	#include <combaseapi.h>
	#include <shlobj.h> /* SHGetKnownFolderPath(), FOLDERID_... */


namespace p
{
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
		if (!files::Exists(path))
		{
			return;
		}

		if (files::IsFolder(path))
		{
			const String fullPath{path};
			::ShellExecuteA(nullptr, "explore", fullPath.data(), nullptr, nullptr, SW_SHOWNORMAL);
		}
		else if (files::IsFile(path))
		{
			String parameters = Strings::Format("/select,{}", path);
			::ShellExecuteA(
			    nullptr, "open", "explorer.exe", parameters.data(), nullptr, SW_SHOWNORMAL);
		}
	}
}    // namespace p
#endif
