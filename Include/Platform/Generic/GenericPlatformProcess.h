// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Export.h"
#include "Strings/StringView.h"


namespace Rift
{
	struct CORE_API GenericPlatformProcess
	{
		/** Content saved to compiler or project directories should be rerouted to user directories
		 * instead **/
		static bool ShouldSaveToUserDir()
		{
			return false;
		}

		static StringView GetBasePath()
		{
			return {};
		}
		// static String GetUserDir() = 0;
		// static String GetUserSettingsDir() = 0;
		// static String GetUserTempDir() = 0;
		// static String GetApplicationSettingsDir() = 0;
	};
}    // namespace Rift
