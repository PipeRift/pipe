// Copyright 2015-2022 Piperift - All rights reserved

#include "Core/Generic/GenericPlatformProcess.h"

#include "Log.h"


namespace p::core
{
	void GenericPlatformProcess::ShowFolder(StringView path)
	{
		Log::Error("ShowFolder not implemented on this platform");
	}
}    // namespace p::core
