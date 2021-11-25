// Copyright 2015-2021 Piperift - All rights reserved

#include "Log.h"


namespace Rift::Checks
{
	void FailedCheckError(const char* msg, sizet size)
	{
		Log::Error(StringView{msg, size});
	}
}    // namespace Rift::Checks
