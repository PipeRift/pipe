// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#if P_PLATFORM_WINDOWS
	#include "Pipe/Core/WindowsPlatformMisc.h"
#elif P_PLATFORM_LINUX
	#include "Pipe/Core/LinuxPlatformMisc.h"
#elif P_PLATFORM_MACOS
	#include "Pipe/Core/MacPlatformMisc.h"
#else
	#error Unknown platform
#endif
