// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Reflect/TypeRegistry.h"


namespace p
{
	Type* TypeRegistry::FindType(TypeId id) const
	{
		if (Type* const* foundTypePtr = idToTypes.Find(id))
		{
			return *foundTypePtr;
		}
		return nullptr;
	}

	TypeRegistry& TypeRegistry::Get()
	{
		static TypeRegistry instance{};
		return instance;
	}
}    // namespace p
