// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Platform/Platform.h"


namespace Rift::Serl
{
	enum class Format : u8
	{
		None = 0,
		Json,
		Bson,
		Binary
	};


	enum Flags
	{
		Flags_None = 0,
		Flags_CheckVersion =
		    1 << 0,    // Version will be written in data. While reading, it will be checked.
		Flags_CheckFlags =
		    1 << 1,    // Flags will be written in data. While reading, they will be checked.
		Flags_NamesAsHashes        = 1 << 2,
		Flags_StoreContainedOwnPtr = 1 << 3    // Store OwnPtr<> instances and their dependant Ptr<>
	};
}    // namespace Rift::Serl
