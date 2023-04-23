// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Core/Log.h"


namespace p::core::Checks
{
	void FailedCheckError(const char* msg, sizet size)
	{
		Error(StringView{msg, size});
	}
}    // namespace p::core::Checks
