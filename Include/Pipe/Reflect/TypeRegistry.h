// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Map.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/MultiLinearArena.h"
#include "Pipe/Reflect/Type.h"
#include "Pipe/Reflect/TypeId.h"
#include "PipeArrays.h"


namespace p
{
	struct TypeFactory
	{
		TypeFactory() {}
	};


	class TypeRegistry
	{
		using ConstIterator = TMap<TypeId, Type*>::ConstIterator;

		// Contains all compiled reflection types linearly in memory
		MultiLinearArena arena{GetCurrentArena()};

		// We map all classes by name in case we need to find them
		TMap<TypeId, Type*> idToTypes;

		TArray<TypeId> ids;
		TArray<i32> parentIndices;

		TArray<TFunction<void()>> compiledTypeRegisters;
		TArray<TypeFactory*> typeFactories;
		bool initialized = false;


	public:
		TypeRegistry();
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

			i32 index = ids.AddSorted(id);
			parentIndices return *ptr;
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

		void Reset();

		PIPE_API void RegisterCompiledType(TFunction<void()> callback);

		static PIPE_API TypeRegistry& Get();


		template<typename T>
		void RegisterType()
		{
			if constexpr (HasSuperMember<T>::value)
			{
				RegisterType<T::Super>();
			}
			TypeId id = GetTypeId<T>();
			i32 index = ids.AddSorted(id);
			parentIndices.InsertRef(index);
		}
	};


	template<typename T>
	struct TTypeFactory : public TypeFactory
	{
		TTypeFactory() {}
	};

	// TTypeFactory<MyType> myTypeFactory{};
}    // namespace p
