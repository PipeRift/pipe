// Copyright 2015-2024 Piperift - All rights reserved

#if P_PLATFORM_WINDOWS
	#include "Pipe/Core/WindowsPlatformMisc.h"

	#include "Pipe/Core/Checks.h"
	#include <cstring>

	#include <Windows.h>


namespace p
{
	const char* WindowsPlatformMisc::GetSystemErrorMessage(char* buffer, i32 size, i32 error)
	{
		P_Check(buffer && size);

		*buffer = '\0';
		if (error == 0)
		{
			error = ::GetLastError();
		}
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error,
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, size, nullptr);
		char* found = std::strchr(buffer, '\r');
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
