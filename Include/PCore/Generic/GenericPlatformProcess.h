// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Export.h"
#include "PCore/StringView.h"



namespace p::core
{
	struct PIPE_API GenericPlatformProcess
	{
		/** Content saved to compiler or project directories should be rerouted to user directories
		 * instead **/
		static bool ShouldSaveToUserDir()
		{
			return false;
		}

		static void ShowFolder(StringView path);
	};
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
