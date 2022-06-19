// Copyright 2015-2022 Piperift - All rights reserved
#include "Pipe/Core/Subprocess.h"

#include "Pipe/Core/Array.h"


namespace p
{
	i32 CreateSubprocess(
	    TSpan<const char* const> command, Subprocess& outProcess, SubprocessOptions options)
	{
		TArray<const char*> checkedCommand;
		checkedCommand.Append(command);
		checkedCommand.Add({nullptr});    // Add nullptr at the end
		return subprocess_create(checkedCommand.Data(), *options, &outProcess.process);
	}
}    // namespace p
