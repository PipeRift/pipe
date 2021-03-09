// Copyright 2015-2021 Piperift - All rights reserved

#include "Reflection/Static/ReflectionRegistry.h"


namespace Rift::Refl
{
	ReflectionRegistry& ReflectionRegistry::Get()
	{
		static ReflectionRegistry instance{};
		return instance;
	}


	void TypeBuilder::Initialize()
	{
		if (initializedType)
		{
			return;
		}

		// Make sure this type has not been initialized before from another builder instance
		// (including DLL static memory)
		initializedType = ReflectionRegistry::Get().FindType(id);
		if (!initializedType)
		{
			initializedType = Build();
		}
	}
}    // namespace Rift::Refl
