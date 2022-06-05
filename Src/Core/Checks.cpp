// Copyright 2015-2022 Piperift - All rights reserved

#include "Log.h"


namespace pipe::core::Checks
{
	void FailedCheckError(const char* msg, sizet size)
	{
		Log::Error(StringView{msg, size});
	}
}    // namespace pipe::core::Checks
