// Copyright 2015-2021 Piperift - All rights reserved

#if PLATFORM_WINDOWS
#	include "Files/Paths.h"
#	include "Platform/Windows/WindowsPlatformProcess.h"
#	include "Strings/String.h"

#	include <Windows.h>


namespace Rift
{
	template <typename StringType, typename TStringGetterFunc>
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

	StringView WindowsPlatformProcess::GetModuleFilePath()
	{
		static const WString path =
		    GetStringFromWindowsAPI<WString>([](WIDECHAR* buffer, int size) {
			    return GetModuleFileNameW(nullptr, buffer, size);
		    });
		WStringView root = fs::_Parse_parent_path(path);
		return {};
	}

	StringView WindowsPlatformProcess::GetBasePath()
	{
		return GetModuleFilePath();
	}
}    // namespace Rift
#endif
