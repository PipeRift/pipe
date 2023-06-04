// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Generic/GenericPlatformProcess.h"

#include "Pipe/Core/Log.h"


namespace p::core
{
	void GenericPlatformProcess::ShowFolder(StringView path)
	{
		Error("ShowFolder not implemented on this platform");
	}
}    // namespace p::core
