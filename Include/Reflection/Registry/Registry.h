// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Map.h"
#include "Memory/Arenas/LinearArena.h"
#include "Reflection/Static/Type.h"
#include "Reflection/TypeId.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	class ReflectionRegistry
	{
		// Contains all compiled reflection types linearly in memory
		Memory::LinearArena arena{256 * 1024};    // First block is 256KB

		// Contains all runtime/data defined types in memory
		// Memory::BestFitArena dynamicArena{256 * 1024};    // First block is 256KB

		// We map all classes by name in case we need to find them
		TMap<TypeId, Type*> idToTypes{};


	public:
		template <typename TType>
		TType& AddType(TypeId id) requires Derived<TType, Type, false>
		{
			TType* ptr = new (arena.Allocate(sizeof(TType))) TType();
			idToTypes.Insert(id, ptr);
			return *ptr;
		}

		CORE_API Type* FindType(TypeId id) const
		{
			if (Type* const* foundTypePtr = idToTypes.Find(id))
			{
				return *foundTypePtr;
			}
			return nullptr;
		}

		template <typename TType>
		void AddProperty()
		{}    // TODO

		CORE_API void* Allocate(sizet size)
		{
			return arena.Allocate(size);
		}

		static CORE_API ReflectionRegistry& Get();
	};
}    // namespace Rift::Refl