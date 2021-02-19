// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Map.h"
#include "Events/Function.h"
#include "Memory/LinearAllocator.h"
#include "Profiler.h"
#include "Reflection/TClass.h"
#include "Reflection/TStruct.h"
#include "Strings/Name.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	class CORE_API ReflectionRegistry
	{
		// Contains all reflection types linearly in memory
		Memory::LinearAllocator<true> allocator{256 * 1024};	// First block is 256KB
		// We map all classes by name in case we need to find them
		TMap<Name, void*> typeIdToInstance{};

	public:
		template <typename T>
		T& AddType(Name uniqueId)
		{
			void* ptr = allocator.Allocate(sizeof(T));
			new (ptr) T();
			typeIdToInstance.Insert(uniqueId, ptr);
			return *static_cast<T*>(ptr);
		}

		Type* FindTypePtr(Name uniqueId) const
		{
			if (void* const* foundTypePtr = typeIdToInstance.Find(uniqueId))
			{
				return static_cast<Type*>(*foundTypePtr);
			}
			return nullptr;
		}

		void* Allocate(size_t size)
		{
			return allocator.Allocate(size);
		}

		static ReflectionRegistry& Get();
	};


	template <typename T, typename Parent, typename TType,
		ReflectionTags tags = ReflectionTags::None>
	struct TTypeBuilder
	{
		static constexpr bool hasParent = !std::is_void_v<Parent>;
		static_assert(Derived<T, BaseStruct>, "Type does not inherit Object or Struct!");
		static_assert(!hasParent || Derived<T, Parent, false>, "Type must derive from parent.");
		static_assert(!(tags & DetailsEdit), "Only properties can use DetailsEdit");
		static_assert(!(tags & DetailsView), "Only properties can use DetailsView");

	protected:
		TType* newType = nullptr;

	public:
		TTypeBuilder(Name uniqueId, Name name, TFunction<void(TTypeBuilder& builder)> onBuild)
		{
			newType = static_cast<TType*>(ReflectionRegistry::Get().FindTypePtr(uniqueId));
			if (!newType)
			{
				Build(uniqueId, name);
				if (onBuild)
				{
					onBuild(*this);
				}
			}
		}

		template <typename PropertyType, ReflectionTags propertyTags>
		void AddProperty(Name name, TFunction<PropertyType*(void*)>&& access)
		{
			ZoneScopedN("AddProperty");
			static_assert(Rift::IsReflectableType<PropertyType>(),
				"PropertyType is not a valid reflected type.");
			static_assert(!(propertyTags & Abstract), "Properties can't be Abstract");


			void* ptr = ReflectionRegistry::Get().Allocate(sizeof(TProperty<PropertyType>));
			auto* const property = new (ptr) TProperty<PropertyType>(
			    newType, GetReflectableName<PropertyType>(), name, Move(access), propertyTags);
			newType->properties.Insert(name, property);
		}

		TType* GetType() const
		{
			return newType;
		}

	protected:
		void Build(Name uniqueId, Name name)
		{
			ZoneScopedN("BuildType");
			if constexpr (hasParent)
			{
				// Parent gets initialized before anything else
				Type* parent = Parent::InitType();

				newType = &ReflectionRegistry::Get().AddType<TType>(uniqueId);
				if (parent)
				{
					newType->parent = parent;
					parent->children.Add(newType);
				}
			}
			else
			{
				newType = &ReflectionRegistry::Get().AddType<TType>(uniqueId);
			}
			newType->name = name;
			newType->tags = tags;
		}
	};
}	 // namespace Rift::Refl
