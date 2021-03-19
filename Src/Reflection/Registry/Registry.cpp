// Copyright 2015-2021 Piperift - All rights reserved

#include "Reflection/Registry/Registry.h"


namespace Rift::Refl
{
	ReflectionRegistry& ReflectionRegistry::Get()
	{
		static ReflectionRegistry instance{};
		return instance;
	}
}    // namespace Rift::Refl
