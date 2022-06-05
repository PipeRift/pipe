// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/StringView.h"
#include "Export.h"


namespace pipe::core
{
	struct CORE_API GenericPlatformProcess
	{
		/** Content saved to compiler or project directories should be rerouted to user directories
		 * instead **/
		static bool ShouldSaveToUserDir()
		{
			return false;
		}

		static void ShowFolder(StringView path);
	};
}    // namespace pipe::core

namespace pipe
{
	using namespace pipe::core;
}
