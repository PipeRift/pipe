// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Map.h"
#include "Memory/Arenas/LinearArena.h"
#include "Reflection/Type.h"
#include "Reflection/TypeId.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	class ReflectionRegistry
	{
		// Contains all compiled reflection types linearly in memory
		Memory::LinearArena arena{256 * 1024};    // First block is 256KB

		// Contains all runtime/data defined types in memory
		// Memory::BigBestFitArena dynamicArena{256 * 1024};    // First block is 256KB

		// We map all classes by name in case we need to find them
		TMap<TypeId, Type*> idToTypes{};


	public:
		template<typename TType>
		TType& AddType(TypeId id) requires Derived<TType, Type, false>
		{
			TType* ptr = new (arena.Allocate(sizeof(TType))) TType();
			idToTypes.Insert(id, ptr);
			return *ptr;
		}


		template<typename PropertyType, typename... Args>
		PropertyType* AddProperty(Args&&... args)
		{
			void* ptr = arena.Allocate(sizeof(PropertyType));
			return new (ptr) PropertyType(std::forward<Args>(args)...);
		}

		CORE_API Type* FindType(TypeId id) const;

		static CORE_API ReflectionRegistry& Get();
	};
}    // namespace Rift::Refl
