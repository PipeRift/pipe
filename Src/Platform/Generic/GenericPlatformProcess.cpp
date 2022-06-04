// Copyright 2015-2022 Piperift - All rights reserved

#include "Platform/Generic/GenericPlatformProcess.h"

#include "Log.h"


namespace Pipe
{
	void GenericPlatformProcess::ShowFolder(StringView path)
	{
		Log::Error("ShowFolder not implemented on this platform");
	}
}    // namespace Pipe
