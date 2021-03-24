// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#if PLATFORM_WINDOWS
#	include "Platform/Windows/WindowsPlatformProcess.h"
#elif PLATFORM_LINUX
#	include "Platform/Linux/LinuxPlatformProcess.h"
#elif PLATFORM_MACOS
#	include "Platform/Mac/MacPlatformProcess.h"
#else
#	error Unknown platform
#endif
