// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#if PLATFORM_WINDOWS
#	include "PCore/Windows/WindowsPlatformProcess.h"
#elif PLATFORM_LINUX
#	include "PCore/Linux/LinuxPlatformProcess.h"
#elif PLATFORM_MACOS
#	include "PCore/Mac/MacPlatformProcess.h"
#else
#	error Unknown platform
#endif
