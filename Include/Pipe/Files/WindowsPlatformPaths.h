// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Export.h"
#include "Pipe/Files/GenericPlatformPaths.h"



namespace p
{
	struct PIPE_API WindowsPlatformPaths : public GenericPlatformPaths
	{
		static u32 GetMaxPathLength();

		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static StringView GetUserPath();
		static StringView GetUserTempPath();
		static StringView GetUserSettingsPath();
		static StringView GetAppSettingsPath();

		static StringView GetCurrentPath();
		static bool SetCurrentPath(StringView path);

		static void ShowFolder(StringView path);


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
				result.reserve(length);
				length = stringGetter(result.data(), result.capacity());
				if (length == 0)
				{
					result.clear();
					break;
				}
				else if (length <= u32(result.capacity()))
				{
					result.resize(length);
					break;
				}
			}
		}
	};
	using PlatformPaths = WindowsPlatformPaths;
}    // namespace p
