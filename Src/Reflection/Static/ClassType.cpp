// Copyright 2015-2021 Piperift - All rights reserved

#include "Reflection/Static/ClassType.h"

#include "Types/ClassBuilder.h"


namespace Rift::Refl
{
	BaseClass* ClassType::CreateInstance()
	{
		if (onCreate)
		{
			return onCreate();
		}
		return {};
	}
}    // namespace Rift::Refl
