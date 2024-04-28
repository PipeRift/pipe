// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Reflect/Builders/TypeBuilder.h"

#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Reflect/TypeRegistry.h"

#include <utility>


namespace p
{
	TypeBuilder::TypeBuilder(TypeId id, StringView name) : id{id}, name{name} {}

	bool TypeBuilder::BeginBuild()
	{
		if (initializedType)
		{
			return false;
		}
		// Make sure this type has not been initialized before from another builder instance
		// (including DLL static memory)
		initializedType = TypeRegistry::Get().FindType(id);
		if (initializedType)
		{
			return false;
		}

		initializedType = CreateType();
		return initializedType != nullptr;
	}

	void TypeBuilder::EndBuild() {}

	TypeId TypeBuilder::GetId() const
	{
		return id;
	}
	StringView TypeBuilder::GetName() const
	{
		return name;
	}
}    // namespace p
