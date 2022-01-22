// Copyright 2015-2022 Piperift - All rights reserved

#if PLATFORM_WINDOWS
#	include "Files/Paths.h"
#	include "Platform/Windows/WindowsPlatformProcess.h"
#	include "Strings/String.h"

#	include <Windows.h>


namespace Rift
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
		return Paths::GetParent(GetExecutableFile());
	}

	StringView WindowsPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}
}    // namespace Rift
#endif
