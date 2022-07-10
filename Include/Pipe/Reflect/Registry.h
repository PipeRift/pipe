// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Map.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/HeapArena.h"
#include "Pipe/Memory/LinearArena.h"
#include "Pipe/Memory/Memory.h"
#include "Pipe/Reflect/Type.h"
#include "Pipe/Reflect/TypeId.h"


namespace p
{
	class ReflectionRegistry
	{
		// Contains all compiled reflection types linearly in memory
		LinearArena arena{};

		// Contains all runtime/data defined types in memory
		// BigBestFitArena dynamicArena{256 * 1024};    // First block is 256KB

		// We map all classes by name in case we need to find them
		TMap<TypeId, Type*> idToTypes{};

		using ConstIterator = TMap<TypeId, Type*>::ConstIterator;


	public:
		~ReflectionRegistry()
		{
			Reset();
		}

		template<typename TType>
		TType& AddType(TypeId id) requires Derived<TType, Type, false>
		{
			auto* ptr = new (p::Alloc<TType>(arena)) TType();
			idToTypes.Insert(id, ptr);
			return *ptr;
		}


		template<typename PropertyType, typename... Args>
		PropertyType* AddProperty(Args&&... args)
		{
			auto* ptr =
			    new (p::Alloc<PropertyType>(arena)) PropertyType(std::forward<Args>(args)...);
			return ptr;
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

		void Reset()
		{
			for (auto it : idToTypes)
			{
				it.second->~Type();
			}
			idToTypes.Empty();
			arena.Reset();
		}

		static PIPE_API ReflectionRegistry& Get();
	};
}    // namespace p
