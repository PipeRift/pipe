// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/Span.h"
#include "Pipe/Core/StringView.h"

#include <subprocessh/subprocess.h>


namespace p
{
	enum class SubprocessOptions
	{
		None = 0,

		// stdout and stderr are the same FILE.
		CombinedStdoutStderr = 1 << 0,

		// The child process should inherit the environment variables of the parent.
		InheritEnvironment = 1 << 1,

		// Enable asynchronous reading of stdout/stderr before it has completed.
		EnableAsync = 1 << 2,

		// Enable the child process to be spawned with no window visible if supported
		// by the platform.
		NoWindow = 1 << 3,

		// Search for program names in the PATH variable. Always enabled on Windows.
		// Note: this will **not** search for paths in any provided custom environment
		// and instead uses the PATH of the spawning process.
		SearchUserPath = InheritEnvironment | NoWindow
	};
	PIPE_DEFINE_FLAG_OPERATORS(SubprocessOptions)


	struct PIPE_API Subprocess
	{
		subprocess_s process;
	};


	PIPE_API i32 CreateSubprocess(TSpan<const char* const> command, Subprocess& outProcess,
	    SubprocessOptions options = SubprocessOptions::None);
}    // namespace p
