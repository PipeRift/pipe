// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#if PLATFORM_WINDOWS
#	include "PCore/Windows/WindowsPlatformMisc.h"
#elif PLATFORM_LINUX
#	include "PCore/Linux/LinuxPlatformMisc.h"
#elif PLATFORM_MACOS
#	include "PCore/Mac/MacPlatformMisc.h"
#else
#	error Unknown platform
#endif
