// Copyright 2015-2021 Piperift - All rights reserved

#include "Reflection/Static/ClassType.h"

#include "Misc/Checks.h"


namespace Rift::Refl
{
	BaseClass* ClassType::New() const
	{
		if (Ensure(onNew))
		{
			return onNew();
		}
		return {};
	}
}    // namespace Rift::Refl
