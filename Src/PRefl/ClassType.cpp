// Copyright 2015-2022 Piperift - All rights reserved

#include "PRefl/ClassType.h"

#include "PCore/Checks.h"


namespace p
{
	BaseClass* ClassType::New() const
	{
		if (Ensure(onNew && !HasFlag(Class_Abstract)))
		{
			return onNew();
		}
		return nullptr;
	}
}    // namespace p
