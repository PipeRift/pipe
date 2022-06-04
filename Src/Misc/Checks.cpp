// Copyright 2015-2022 Piperift - All rights reserved

#include "Log.h"


namespace Pipe::Checks
{
	void FailedCheckError(const char* msg, sizet size)
	{
		Log::Error(StringView{msg, size});
	}
}    // namespace Pipe::Checks
