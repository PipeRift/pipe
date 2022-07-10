// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Reflect/ClassType.h"

#include "Pipe/Core/Checks.h"


namespace p
{
	BaseClass* ClassType::New(Arena& arena) const
	{
		if (Ensure(onNew && !HasFlag(Class_Abstract)))
		{
			return onNew(arena);
		}
		return nullptr;
	}
}    // namespace p
