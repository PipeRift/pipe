// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#if P_PLATFORM_WINDOWS
#	include "Pipe/Core/Windows/WindowsPlatformMisc.h"
#elif P_PLATFORM_LINUX
#	include "Pipe/Core/Linux/LinuxPlatformMisc.h"
#elif P_PLATFORM_MACOS
#	include "Pipe/Core/Mac/MacPlatformMisc.h"
#else
#	error Unknown platform
#endif
