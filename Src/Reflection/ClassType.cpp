// Copyright 2015-2022 Piperift - All rights reserved

#include "Reflection/ClassType.h"

#include "Core/Checks.h"


namespace pipe::refl
{
	BaseClass* ClassType::New() const
	{
		if (Ensure(onNew && !HasFlag(Class_Abstract)))
		{
			return onNew();
		}
		return nullptr;
	}
}    // namespace pipe::refl
