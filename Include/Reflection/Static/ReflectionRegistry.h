// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Map.h"
#include "Events/Function.h"
#include "Memory/Arenas/LinearArena.h"
#include "Profiler.h"
#include "Reflection/Static/BaseType.h"
#include "Reflection/Static/Class.h"
#include "Reflection/Static/EnumType.h"
#include "Reflection/Static/Struct.h"
#include "Reflection/TypeId.h"
#include "Strings/Name.h"
#include "Strings/String.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	template <typename T>
	struct TTypeInstance
	{
		static BaseType* instance;


		static Type* InitType() requires(IsClass<T>() || IsStruct<T>())
		{
			return T::InitType();
		}
	};

	template <typename T>
	inline BaseType* TTypeInstance<T>::instance = TTypeInstance<T>::InitType();


	/////////////////////////////////////////////////////////////////
	//  Type Builders & Registry

	class CORE_API ReflectionRegistry
	{
		// Contains all compiled reflection types linearly in memory
		Memory::LinearArena arena{256 * 1024};    // First block is 256KB

		// Contains all runtime/data defined types in memory
		// Memory::BestFitArena dynamicArena{256 * 1024};    // First block is 256KB

		// We map all classes by name in case we need to find them
		TMap<TypeId, BaseType*> idToTypes{};


	public:
		template <typename TType>
		TType& AddType(TypeId id) requires Derived<TType, BaseType, false>
		{
			TType* ptr = new (arena.Allocate(sizeof(TType))) TType();
			idToTypes.Insert(id, ptr);
			return *ptr;
		}

		BaseType* FindType(TypeId id) const
		{
			if (BaseType* const* foundTypePtr = idToTypes.Find(id))
			{
				return *foundTypePtr;
			}
			return nullptr;
		}

		void* Allocate(sizet size)
		{
			return arena.Allocate(size);
		}

		static ReflectionRegistry& Get();
	};


	struct CORE_API BaseTypeBuilder
	{
	protected:
		TypeId id;
		Name name;
		BaseType* initializedType = nullptr;


	public:
		BaseTypeBuilder(TypeId id, Name name) : id{id}, name{name} {}
		virtual ~BaseTypeBuilder() {}

		void Initialize();

		TypeId GetId() const
		{
			return id;
		}
		Name GetName() const
		{
			return name;
		}

	protected:
		virtual BaseType* Build() = 0;
	};


	template <typename T, typename Parent, typename TType,
	    ReflectionTags tags = ReflectionTags::None>
	struct TTypeBuilder : public BaseTypeBuilder
	{
		static constexpr bool hasParent = !std::is_void_v<Parent>;
		static_assert(!hasParent || Derived<T, Parent, false>, "Type must derive from parent.");
		static_assert(!(tags & DetailsEdit), "Only properties can use DetailsEdit");
		static_assert(!(tags & DetailsView), "Only properties can use DetailsView");

	public:
		TTypeBuilder(Name name) : BaseTypeBuilder(TypeId::Get<T>(), name) {}

		template <typename PropertyType, ReflectionTags propertyTags>
		void AddProperty(Name name, TFunction<PropertyType*(void*)>&& access)
		{
			static_assert(
			    Rift::IsReflected<PropertyType>(), "PropertyType is not a valid reflected type.");
			static_assert(!(propertyTags & Abstract), "Properties can't be Abstract");


			void* ptr = ReflectionRegistry::Get().Allocate(sizeof(TProperty<PropertyType>));

			auto* const property = new (ptr) TProperty<PropertyType>(GetComposeType(),
			    GetReflectedName<PropertyType>(), name, Move(access), propertyTags);

			GetComposeType()->properties.Insert(name, property);
		}

		Type* GetComposeType() const
		{
			return static_cast<Type*>(initializedType);
		}

	protected:
		BaseType* Build() override
		{
			TType* newType;
			if constexpr (hasParent)
			{
				// Parent gets initialized before anything else
				Type* parent = TTypeInstance<Parent>::InitType();
				assert(parent);

				newType = &ReflectionRegistry::Get().AddType<TType>(GetId());
				parent->children.Add(newType);
				newType->parent = parent;
			}
			else
			{
				newType = &ReflectionRegistry::Get().AddType<TType>(GetId());
			}
			newType->name = name;
			newType->tags = tags;

			initializedType = newType;
			return newType;
		}
	};

	template <typename T, typename Parent, ReflectionTags tags = ReflectionTags::None>
	struct TClassBuilder : public TTypeBuilder<T, Parent, Class, tags>
	{
		static_assert(IsClass<T>(), "Type does not inherit Object!");
		using Super     = TTypeBuilder<T, Parent, Class, tags>;
		using BuildFunc = TFunction<void(TClassBuilder& builder)>;

		BuildFunc onBuild;


	public:
		TClassBuilder(Name name) : Super(name) {}

		Class* GetType() const
		{
			return static_cast<Class*>(initializedType);
		}

	protected:
		BaseType* Build() override
		{
			auto* type = Super::Build();

			GetType()->onCreate = [](const auto& owner) {
				if constexpr (IsSame<T, BaseObject>)
				{
					return OwnPtr<T, ObjectBuilder<T>>{};    // Can't create instances of BaseObject
				}
				return MakeOwned<T, ObjectBuilder<T>>(owner);
			};

			if (onBuild)
			{
				onBuild(*this);
			}
			return type;
		}
	};


	template <typename T, typename Parent, ReflectionTags tags = ReflectionTags::None>
	struct TStructBuilder : public TTypeBuilder<T, Parent, Struct, tags>
	{
		static_assert(IsStruct<T>(), "Type does not inherit Struct!");

		using Super     = TTypeBuilder<T, Parent, Struct, tags>;
		using BuildFunc = TFunction<void(TStructBuilder& builder)>;

		BuildFunc onBuild;


	public:
		TStructBuilder(Name name) : Super(name) {}

		Struct* GetType() const
		{
			return static_cast<Struct*>(initializedType);
		}

	protected:
		BaseType* Build() override
		{
			auto* type = Super::Build();
			if (onBuild)
			{
				onBuild(*this);
			}
			return type;
		}
	};


	/**
	 * Enum Type Builder
	 * Builds enum types during static initialization
	 */
	template <typename T>
	struct TEnumTypeBuilder : public BaseTypeBuilder
	{
	public:
		TEnumTypeBuilder(Name name) : BaseTypeBuilder(TypeId::Get<T>(), name)
		{
			Initialize();
		}

		EnumType* GetType() const
		{
			return static_cast<EnumType*>(initializedType);
		}

	protected:
		BaseType* Build() override
		{
			EnumType& newType = ReflectionRegistry::Get().AddType<EnumType>(GetId());
			newType.name      = name;
			return &newType;
		}
	};
}    // namespace Rift::Refl
