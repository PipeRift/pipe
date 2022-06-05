// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Generic/GenericPlatformProcess.h"
#include "Export.h"


namespace p::core
{
	struct CORE_API WindowsPlatformProcess : public GenericPlatformProcess
	{
		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static void ShowFolder(StringView path);
	};

	using PlatformProcess = WindowsPlatformProcess;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
