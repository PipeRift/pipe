// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Profiler.h"
#include "Reflection/Registry/Registry.h"
#include "Reflection/Registry/TypeBuilder.h"
#include "Reflection/Static/ClassType.h"
#include "Reflection/Static/EnumType.h"
#include "Reflection/Static/StructType.h"
#include "Reflection/Static/Type.h"
#include "Reflection/TypeId.h"
#include "Reflection/TypeName.h"
#include "Strings/Name.h"
#include "Strings/String.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	template <typename T>
	struct TTypeInstance
	{
		static Type* instance;


		static DataType* InitType() requires(IsClass<T>() || IsStruct<T>())
		{
			return T::InitType();
		}

		static EnumType* InitType() requires(IsEnum<T>())
		{
			return TStaticEnumInitializer<T>::onInit();
		}
	};

	template <typename T>
	inline Type* TTypeInstance<T>::instance = TTypeInstance<T>::InitType();


	/////////////////////////////////////////////////////////////////
	//  Type Builders

	template <typename T, typename Parent, typename TType,
	    ReflectionTags tags = ReflectionTags::None>
	struct TDataTypeBuilder : public TypeBuilder
	{
		static constexpr bool hasParent = !std::is_void_v<Parent>;
		static_assert(!hasParent || Derived<T, Parent, false>, "Type must derive from parent.");
		static_assert(!(tags & DetailsEdit), "Only properties can use DetailsEdit");
		static_assert(!(tags & DetailsView), "Only properties can use DetailsView");

	public:
		TDataTypeBuilder() = default;
		TDataTypeBuilder(Name name) : TypeBuilder(TypeId::Get<T>(), name) {}

		template <typename PropertyType, ReflectionTags propertyTags>
		void AddProperty(Name name, TFunction<PropertyType*(void*)>&& access)
		{
			static_assert(HasType<PropertyType>(), "PropertyType is not a reflected type.");
			static_assert(!(propertyTags & Abstract), "Properties can't be Abstract");


			void* ptr = ReflectionRegistry::Get().Allocate(sizeof(TProperty<PropertyType>));

			auto* const property = new (ptr) TProperty<PropertyType>(
			    GetType(), GetTypeName<PropertyType>(), name, Move(access), propertyTags);

			GetType()->properties.Insert(name, property);
		}

		TType* GetType() const
		{
			return static_cast<TType*>(initializedType);
		}

	protected:
		Type* Build() override
		{
			TType* newType;
			if constexpr (hasParent)
			{
				// Parent gets initialized before anything else
				DataType* parent = TTypeInstance<Parent>::InitType();
				assert(parent);

				newType = &ReflectionRegistry::Get().AddType<TType>(GetId());
				parent->children.Add(newType);
				newType->parent = parent;
			}
			else
			{
				newType = &ReflectionRegistry::Get().AddType<TType>(GetId());
			}
			newType->id   = id;
			newType->name = name;
			newType->tags = tags;

			initializedType = newType;
			return newType;
		}
	};

	template <typename T, typename Parent, ReflectionTags tags = ReflectionTags::None>
	struct TClassTypeBuilder : public TDataTypeBuilder<T, Parent, ClassType, tags>
	{
		static_assert(IsClass<T>(), "Type does not inherit Object!");
		using Super     = TDataTypeBuilder<T, Parent, ClassType, tags>;
		using BuildFunc = TFunction<void(TClassTypeBuilder& builder)>;
		using Super::GetType;

		BuildFunc onBuild;


	public:
		TClassTypeBuilder() = default;
		TClassTypeBuilder(Name name) : Super(name) {}

	protected:
		Type* Build() override
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
	struct TStructTypeBuilder : public TDataTypeBuilder<T, Parent, StructType, tags>
	{
		static_assert(IsStruct<T>(), "Type does not inherit Struct!");

		using Super     = TDataTypeBuilder<T, Parent, StructType, tags>;
		using BuildFunc = TFunction<void(TStructTypeBuilder& builder)>;
		using Super::GetType;

		BuildFunc onBuild;


	public:
		TStructTypeBuilder() = default;
		TStructTypeBuilder(Name name) : Super(name) {}

	protected:
		Type* Build() override
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
	struct TNativeTypeBuilder : public TypeBuilder
	{
	public:
		TNativeTypeBuilder() = default;
		TNativeTypeBuilder(Name name) : TypeBuilder(TypeId::Get<T>(), name) {}

		EnumType* GetType() const
		{
			return static_cast<EnumType*>(initializedType);
		}

	protected:
		Type* Build() override
		{
			NativeType& newType = ReflectionRegistry::Get().AddType<NativeType>(GetId());

			newType.id   = id;
			newType.name = name;
			return &newType;
		}
	};
}    // namespace Rift::Refl
