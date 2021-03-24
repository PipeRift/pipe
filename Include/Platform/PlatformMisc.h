// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#if PLATFORM_WINDOWS
#	include "Platform/Windows/WindowsPlatformMisc.h"
#elif PLATFORM_LINUX
#	include "Platform/Linux/LinuxPlatformMisc.h"
#elif PLATFORM_MACOS
#	include "Platform/Mac/MacPlatformMisc.h"
#else
#	error Unknown platform
#endif
