// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Checks.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Core/StringView.h"


namespace p::internal
{
	void FailedCheckError(const char* msg, sizet size)
	{
		Error(StringView{msg, size});
	}
}    // namespace p::internal
