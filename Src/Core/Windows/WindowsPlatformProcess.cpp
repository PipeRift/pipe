// Copyright 2015-2023 Piperift - All rights reserved

#if P_PLATFORM_WINDOWS
	#include "Pipe/Core/Windows/WindowsPlatformProcess.h"

	#include "Pipe/Files/Paths.h"
	#include "Pipe/Files/Files.h"
	#include "Pipe/Core/Platform.h"
	#include "Pipe/Core/Windows/WindowsPlatformMisc.h"
	#include "Pipe/Core/String.h"
	#include "Pipe/Core/Log.h"

	#include <Windows.h>
	#include <combaseapi.h>
	#include <shlobj.h> /* SHGetKnownFolderPath(), FOLDERID_... */


namespace p
{
	template<typename StringType, typename TStringGetterFunc>
	StringType GetStringFromWindowsAPI(TStringGetterFunc stringGetter, int initialSize = MAX_PATH)
	{
		if (initialSize <= 0)
		{
			initialSize = MAX_PATH;
		}

		StringType result(initialSize, 0);
		for (;;)
		{
			auto length = stringGetter(result.data(), result.length());
			if (length == 0)
			{
				return {};
			}

			if (length < result.length() - 1)
			{
				result.resize(length);
				result.shrink_to_fit();
				return result;
			}

			result.resize(result.length() * 2);
		}
	}

	StringView WindowsPlatformProcess::GetExecutableFile()
	{
		static const auto filePath = GetStringFromWindowsAPI<String>([](TChar* buffer, sizet size) {
	#if PLATFORM_TCHAR_IS_WCHAR
			return GetModuleFileNameW(nullptr, buffer, u32(size));
	#else
			return GetModuleFileNameA(nullptr, buffer, u32(size));
	#endif
		});
		return filePath;
	}

	StringView WindowsPlatformProcess::GetExecutablePath()
	{
		return GetParentPath(GetExecutableFile());
	}

	StringView WindowsPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}

	StringView WindowsPlatformProcess::GetUserPath()
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

	StringView WindowsPlatformProcess::GetUserTempPath()
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

	StringView WindowsPlatformProcess::GetUserSettingsPath()
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

	StringView WindowsPlatformProcess::GetAppSettingsPath()
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

	String WindowsPlatformProcess::GetCurrentWorkingPath()
	{
		String buffer;
		// Loop in case the variable changes while running, or the buffer isn't large
		// enough.
		for (u32 length = 128;;)
		{
			buffer.resize(length);
	#if PLATFORM_TCHAR_IS_WCHAR
			length = ::GetCurrentDirectoryW(buffer.capacity(), buffer.data());
	#else
			length = ::GetCurrentDirectoryA(buffer.capacity(), buffer.data());
	#endif
			if (length == 0)
			{
				buffer.clear();
				break;
			}
			else if (length < u32(buffer.size()))
			{
				buffer.resize(length);
				break;
			}
		}
		return Move(buffer);
	}

	bool WindowsPlatformProcess::SetCurrentWorkingPath(StringView path)
	{
	#if PLATFORM_TCHAR_IS_WCHAR
		return ::SetCurrentDirectoryW(path.data());
	#else
		return ::SetCurrentDirectoryA(path.data());
	#endif
	}


	void WindowsPlatformProcess::ShowFolder(StringView path)
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
