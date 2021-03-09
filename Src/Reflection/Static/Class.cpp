// Copyright 2015-2021 Piperift - All rights reserved

#include "Reflection/Static/Class.h"

#include "Object/ObjectBuilder.h"


namespace Rift::Refl
{
	OwnPtr<BaseObject, ObjectBuilder<BaseObject>> ClassType::CreateInstance(
	    const Ptr<BaseObject>& owner)
	{
		if (onCreate)
		{
			return onCreate(owner);
		}
		return {};
	}
}    // namespace Rift::Refl
