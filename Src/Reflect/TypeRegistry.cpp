// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Reflect/TypeRegistry.h"


namespace p
{
	void TypeRegistry::Initialize()
	{
		auto& registry = Get();
		if (!EnsureMsg(!registry.initialized, "Reflection is already initialized"))
		{
			return;
		}

		auto& callbacks = registry.compiledTypeRegisters;
		for (auto callback : callbacks)
		{
			callback();
		}
		callbacks.Clear();
		registry.initialized = true;
	}

	Type* TypeRegistry::FindType(TypeId id) const
	{
		if (Type* const* foundTypePtr = idToTypes.Find(id))
		{
			return *foundTypePtr;
		}
		return nullptr;
	}

	void TypeRegistry::RegisterCompiledType(TFunction<void()> callback)
	{
		if (EnsureMsg(!initialized, "Can't register compile types after initializing!"))
		{
			compiledTypeRegisters.Add(callback);
		}
	}

	TypeRegistry& TypeRegistry::Get()
	{
		static TypeRegistry instance{};
		return instance;
	}
}    // namespace p
