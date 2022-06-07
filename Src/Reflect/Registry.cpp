// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Reflect/Registry.h"


namespace p
{
	Type* ReflectionRegistry::FindType(TypeId id) const
	{
		if (Type* const* foundTypePtr = idToTypes.Find(id))
		{
			return *foundTypePtr;
		}
		return nullptr;
	}

	ReflectionRegistry& ReflectionRegistry::Get()
	{
		static ReflectionRegistry instance{};
		return instance;
	}
}    // namespace p
