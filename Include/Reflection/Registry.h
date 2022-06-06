// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Core/Map.h"
#include "Core/TypeTraits.h"
#include "Memory/Arenas/LinearArena.h"
#include "Reflection/Type.h"
#include "Reflection/TypeId.h"



namespace p
{
	class ReflectionRegistry
	{
		// Contains all compiled reflection types linearly in memory
		Memory::LinearArena arena{256 * 1024};    // First block is 256KB

		// Contains all runtime/data defined types in memory
		// Memory::BigBestFitArena dynamicArena{256 * 1024};    // First block is 256KB

		// We map all classes by name in case we need to find them
		TMap<TypeId, Type*> idToTypes{};

		using ConstIterator = TMap<TypeId, Type*>::ConstIterator;


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

		PIPE_API Type* FindType(TypeId id) const;


		PIPE_API ConstIterator begin() const
		{
			return idToTypes.begin();
		}
		PIPE_API ConstIterator end() const
		{
			return idToTypes.end();
		}


		static PIPE_API ReflectionRegistry& Get();
	};
}    // namespace p
