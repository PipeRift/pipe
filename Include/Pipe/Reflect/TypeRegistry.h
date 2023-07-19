// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Map.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/MultiLinearArena.h"
#include "Pipe/PipeArrays.h"
#include "Pipe/Reflect/Type.h"
#include "Pipe/Reflect/TypeId.h"


namespace p
{
	class TypeRegistry
	{
		using ConstIterator = TMap<TypeId, Type*>::ConstIterator;

		// Contains all compiled reflection types linearly in memory
		MultiLinearArena arena{GetCurrentArena()};

		// We map all classes by name in case we need to find them
		TMap<TypeId, Type*> idToTypes{};

		TArray<TFunction<void()>> compiledTypeRegisters;
		bool initialized = false;


	public:
		~TypeRegistry()
		{
			Reset();
		}

		static PIPE_API void Initialize();


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
			idToTypes.Clear();
			arena.Release();
		}

		PIPE_API void RegisterCompiledType(TFunction<void()> callback);

		static PIPE_API TypeRegistry& Get();
	};
}    // namespace p
