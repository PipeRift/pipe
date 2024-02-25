// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#if P_PLATFORM_WINDOWS
	#include "Pipe/Files/WindowsPlatformPaths.h"
#elif P_PLATFORM_LINUX
	#include "Pipe/Files/LinuxPlatformPaths.h"
#elif P_PLATFORM_MACOS
	#include "Pipe/Files/MacPlatformPaths.h"
#else
	#error Unknown platform
#endif
