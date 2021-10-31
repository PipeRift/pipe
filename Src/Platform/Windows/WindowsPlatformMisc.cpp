// Copyright 2015-2021 Piperift - All rights reserved

#if PLATFORM_WINDOWS
#	include "Platform/Windows/WindowsPlatformMisc.h"

#	include <Windows.h>


namespace Rift
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

	static u32 GetMaxPathLength()
	{
		static const u32 maxPath = AreLongPathsEnabled() ? 32767 : MAX_PATH;
		return maxPath;
	}
}    // namespace Rift
#endif
