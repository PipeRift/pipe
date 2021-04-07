// Copyright 2015-2021 Piperift - All rights reserved

#include "Reflection/Static/ClassType.h"

#include "Object/ObjectBuilder.h"


namespace Rift::Refl
{
	TOwnPtr<BaseObject, ObjectBuilder<BaseObject>> ClassType::CreateInstance(
	    const TPtr<BaseObject>& owner)
	{
		if (onCreate)
		{
			return onCreate(owner);
		}
		return {};
	}
}    // namespace Rift::Refl
