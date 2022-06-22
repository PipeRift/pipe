// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Span.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Export.h"


namespace p::core
{
	struct PIPE_API GenericPlatformProcess
	{
		static StringView GetExecutableFile() = delete;
		static StringView GetExecutablePath() = delete;
		static StringView GetBasePath()       = delete;

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
