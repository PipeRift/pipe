// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Reflect/TypeRegistry.h"

#include "Pipe/Memory/HeapArena.h"


namespace p
{
	TypeRegistry::TypeRegistry() : idToTypes{GetHeapArena()} {}

	void TypeRegistry::Initialize()
	{
		auto& registry = Get();
		if (!P_EnsureMsg(!registry.initialized, "Reflection is already initialized"))
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
		if (P_EnsureMsg(!initialized, "Can't register compile types after initializing!"))
		{
			compiledTypeRegisters.Add(callback);
		}
	}

	void TypeRegistry::Reset()
	{
		for (auto it : idToTypes)
		{
			it.second->~Type();
		}
		idToTypes = {GetHeapArena()};
		arena.Release();
	}

	TypeRegistry& TypeRegistry::Get()
	{
		static TypeRegistry instance{};
		return instance;
	}
}    // namespace p
