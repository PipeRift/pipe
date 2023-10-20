// Copyright 2015-2023 Piperift - All rights reserved

#if P_PLATFORM_WINDOWS
	#include "Pipe/Core/Windows/WindowsPlatformMisc.h"

	#include "Pipe/Core/Checks.h"
	#include <cstring>

	#include <Windows.h>


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

	u32 WindowsPlatformMisc::GetMaxPathLength()
	{
		static const u32 maxPath = AreLongPathsEnabled() ? 32767 : MAX_PATH;
		return maxPath;
	}

	const TChar* WindowsPlatformMisc::GetSystemErrorMessage(TChar* buffer, i32 size, i32 error)
	{
		Check(buffer && size);

		*buffer = '\0';
		if (error == 0)
		{
			error = ::GetLastError();
		}
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error,
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, size, nullptr);
		TChar* found = std::strchr(buffer, '\r');
		if (found)
		{
			*found = '\0';
		}
		found = std::strchr(buffer, '\n');
		if (found)
		{
			*found = '\0';
		}
		return buffer;
	}
}    // namespace p
#endif
