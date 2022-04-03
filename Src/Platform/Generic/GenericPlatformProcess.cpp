// Copyright 2015-2022 Piperift - All rights reserved

#include "Platform/Generic/GenericPlatformProcess.h"

#include "Log.h"


namespace Rift
{
	void GenericPlatformProcess::ShowFolder(StringView path)
	{
		Log::Error("ShowFolder not implemented on this platform");
	}
}    // namespace Rift
