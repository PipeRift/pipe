// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Platform.h"
#if P_PLATFORM_WINDOWS
#	include "Pipe/Files/Paths.h"
#	include "Pipe/Files/Files.h"
#	include "Pipe/Core/Windows/WindowsPlatformProcess.h"
#	include "Pipe/Core/String.h"
#	include "Pipe/Core/Log.h"

#	include <Windows.h>


namespace p::core
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
#	if PLATFORM_TCHAR_IS_WCHAR
			return GetModuleFileNameW(nullptr, buffer, u32(size));
#	else
			return GetModuleFileNameA(nullptr, buffer, u32(size));
#	endif
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
}    // namespace p::core
#endif
