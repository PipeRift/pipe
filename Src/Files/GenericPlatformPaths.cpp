// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Log.h"
#include "Pipe/Files/PlatformPaths.h"



namespace p
{
	StringView GenericPlatformPaths::GetUserPath()
	{
		// default to the base directory
		return PlatformPaths::GetBasePath();
	}

	StringView GenericPlatformPaths::GetUserTempPath()
	{
		// default to the base directory
		return PlatformPaths::GetBasePath();
	}

	StringView GenericPlatformPaths::GetUserSettingsPath()
	{
		// default to the base directory
		return PlatformPaths::GetBasePath();
	}

	StringView GenericPlatformPaths::GetAppSettingsPath()
	{
		// default to the base directory
		return PlatformPaths::GetBasePath();
	}

	void GenericPlatformPaths::ShowFolder(StringView path)
	{
		Error("ShowFolder not implemented on this platform");
	}
}    // namespace p
