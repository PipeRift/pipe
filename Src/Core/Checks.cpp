// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Log.h"


namespace p::internal
{
	void FailedCheckError(const char* msg, sizet size)
	{
		Error(StringView{msg, size});
	}
}    // namespace p::internal
