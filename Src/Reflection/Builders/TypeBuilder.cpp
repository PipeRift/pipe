// Copyright 2015-2022 Piperift - All rights reserved

#include "Reflection/Builders/TypeBuilder.h"

#include "Reflection/Registry.h"
#include "Reflection/TypeId.h"

#include <utility>


namespace p
{
	TypeBuilder::TypeBuilder(TypeId id, StringView name) : id{id}, name{name} {}

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

	TypeId TypeBuilder::GetId() const
	{
		return id;
	}
	StringView TypeBuilder::GetName() const
	{
		return name;
	}
}    // namespace p
