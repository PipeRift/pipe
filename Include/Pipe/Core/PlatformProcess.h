// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#if PLATFORM_WINDOWS
#	include "Pipe/Core/Windows/WindowsPlatformProcess.h"
#elif PLATFORM_LINUX
#	include "Pipe/Core/Linux/LinuxPlatformProcess.h"
#elif PLATFORM_MACOS
#	include "Pipe/Core/Mac/MacPlatformProcess.h"
#else
#	error Unknown platform
#endif
