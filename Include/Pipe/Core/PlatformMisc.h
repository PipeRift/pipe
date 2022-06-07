// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#if PLATFORM_WINDOWS
#	include "Pipe/Core/Windows/WindowsPlatformMisc.h"
#elif PLATFORM_LINUX
#	include "Pipe/Core/Linux/LinuxPlatformMisc.h"
#elif PLATFORM_MACOS
#	include "Pipe/Core/Mac/MacPlatformMisc.h"
#else
#	error Unknown platform
#endif
