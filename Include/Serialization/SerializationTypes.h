// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"


namespace Rift::Serl
{
	enum Flags
	{
		Flags_None = 0,
		Flags_CheckVersion =
		    1 << 0,    // Version will be saved while writing. While reading, it will be checked.
		Flags_CheckFlags =
		    1 << 1,    // Flags will be saved while writing. While reading, they will be checked.
		Flags_NamesAsHashes = 1 << 2
	};
}    // namespace Rift::Serl
