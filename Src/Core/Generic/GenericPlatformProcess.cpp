// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Generic/GenericPlatformProcess.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Core/PlatformProcess.h"


namespace p::core
{
	StringView GenericPlatformProcess::GetUserPath()
	{
		// default to the base directory
		return PlatformProcess::GetBasePath();
	}

	StringView GenericPlatformProcess::GetUserSettingsPath()
	{
		// default to the base directory
		return PlatformProcess::GetBasePath();
	}

	StringView GenericPlatformProcess::GetAppSettingsPath()
	{
		// default to the base directory
		return PlatformProcess::GetBasePath();
	}

	void GenericPlatformProcess::ShowFolder(StringView path)
	{
		Error("ShowFolder not implemented on this platform");
	}
}    // namespace p::core
