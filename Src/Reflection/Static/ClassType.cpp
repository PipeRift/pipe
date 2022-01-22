// Copyright 2015-2022 Piperift - All rights reserved

#include "Reflection/Static/ClassType.h"

#include "Misc/Checks.h"


namespace Rift::Refl
{
	BaseClass* ClassType::New() const
	{
		if (Ensure(onNew && !HasFlag(Class_Abstract)))
		{
			return onNew();
		}
		return nullptr;
	}
}    // namespace Rift::Refl
