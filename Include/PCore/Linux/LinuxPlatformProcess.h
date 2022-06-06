// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Export.h"
#include "PCore/Generic/GenericPlatformProcess.h"



namespace p::core
{
	struct PIPE_API LinuxPlatformProcess : public GenericPlatformProcess
	{
		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static void ShowFolder(StringView path);
	};

	using PlatformProcess = LinuxPlatformProcess;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
