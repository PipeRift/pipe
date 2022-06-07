// Copyright 2015-2022 Piperift - All rights reserved

#if PLATFORM_WINDOWS
#	include "Pipe/Core/Windows/WindowsPlatformMisc.h"

#	include <Windows.h>


namespace p::core
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

	u32 WindowsPlatformMisc::GetMaxPathLength()
	{
		static const u32 maxPath = AreLongPathsEnabled() ? 32767 : MAX_PATH;
		return maxPath;
	}
}    // namespace p::core
#endif
