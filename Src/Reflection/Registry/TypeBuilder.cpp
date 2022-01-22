// Copyright 2015-2022 Piperift - All rights reserved

#include "Reflection/Registry/TypeBuilder.h"

#include "Reflection/Registry/Registry.h"
#include "Reflection/TypeId.h"

#include <utility>


namespace Rift::Refl
{
	TypeBuilder::TypeBuilder(TypeId id, Name name) : id{id}, name{std::move(name)} {}

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
	Name TypeBuilder::GetName() const
	{
		return name;
	}
}    // namespace Rift::Refl
