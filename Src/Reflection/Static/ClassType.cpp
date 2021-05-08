// Copyright 2015-2021 Piperift - All rights reserved

#include "Reflection/Static/ClassType.h"

#include "Object/ObjectBuilder.h"


namespace Rift::Refl
{
	BaseObject* ClassType::CreateInstance()
	{
		if (onCreate)
		{
			return onCreate();
		}
		return {};
	}
}    // namespace Rift::Refl
