// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Events/Function.h"
#include "Reflection/Registry/Registry.h"
#include "Reflection/Registry/TypeBuilder.h"
#include "Reflection/Static/ClassType.h"
#include "Reflection/Static/StructType.h"
#include "Reflection/TypeId.h"
#include "Strings/Name.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
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
				DataType* parent = (DataType*) TTypeInstance<Parent>::InitType();
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


	// Wrap some types so we can use them at compile time
	template <u32 N>
	struct MetaCounter : MetaCounter<N - 1>
	{
		static constexpr u32 value = N;
	};
	template <>
	struct MetaCounter<0>
	{
		static constexpr u32 value = 0;
	};
}    // namespace Rift::Refl


#define CLASS(...) TYPE_CHOOSER(__CLASS_NO_TAGS, __CLASS_TAGS, __VA_ARGS__)(__VA_ARGS__)
#define STRUCT(...) TYPE_CHOOSER(__STRUCT_NO_TAGS, __STRUCT_TAGS, __VA_ARGS__)(__VA_ARGS__)
#define PROP(...) TYPE_CHOOSER(__PROPERTY_NO_TAGS, __PROPERTY_TAGS, __VA_ARGS__)(__VA_ARGS__)


/** Defines a class with no parent */
#define ORPHAN_CLASS(type, tags)                                             \
public:                                                                      \
	using ThisType    = type;                                                \
	using BuilderType = Rift::Refl::TClassTypeBuilder<ThisType, void, tags>; \
                                                                             \
	virtual Rift::Refl::ClassType* GetClass() const                          \
	{                                                                        \
		return Rift::GetType<ThisType>();                                    \
	}                                                                        \
	virtual void SerializeReflection(Rift::Archive& ar)                      \
	{                                                                        \
		__ReflSerializeProperty(ar, Rift::Refl::MetaCounter<0>{});           \
	}                                                                        \
	BASECLASS(type)


/** Defines a Class */
#define __CLASS_NO_TAGS(type, parent) __CLASS_TAGS(type, parent, Rift::ReflectionTags::None)
#define __CLASS_TAGS(type, parent, tags)                                      \
public:                                                                       \
	using ThisType    = type;                                                 \
	using Super       = parent;                                               \
	using BuilderType = Rift::Refl::TClassTypeBuilder<ThisType, Super, tags>; \
                                                                              \
	Rift::Refl::ClassType* GetClass() const override                          \
	{                                                                         \
		return Rift::GetType<ThisType>();                                     \
	}                                                                         \
	void SerializeReflection(Rift::Archive& ar) override                      \
	{                                                                         \
		Super::SerializeReflection(ar);                                       \
		__ReflSerializeProperty(ar, Rift::Refl::MetaCounter<0>{});            \
	}                                                                         \
	BASECLASS(type)


#define BASECLASS(type)                                                                     \
public:                                                                                     \
	static Rift::Refl::ClassType* InitType()                                                \
	{                                                                                       \
		BuilderType builder{Rift::Name{TX(#type)}};                                         \
		builder.onBuild = [](auto& builder) {                                               \
			__ReflReflectProperty(builder, Rift::Refl::MetaCounter<0>{});                   \
		};                                                                                  \
		builder.Initialize();                                                               \
		return builder.GetType();                                                           \
	}                                                                                       \
                                                                                            \
private:                                                                                    \
	static constexpr Rift::Refl::MetaCounter<0> __refl_Counter(Rift::Refl::MetaCounter<0>); \
	template <Rift::u32 N>                                                                  \
	static void __ReflReflectProperty(BuilderType&, Rift::Refl::MetaCounter<N>)             \
	{}                                                                                      \
	template <Rift::u32 N>                                                                  \
	void __ReflSerializeProperty(Rift::Archive&, Rift::Refl::MetaCounter<N>)                \
	{}


/** Defines an struct with no parent */
#define ORPHAN_STRUCT(type, tags)                                             \
public:                                                                       \
	using ThisType    = type;                                                 \
	using BuilderType = Rift::Refl::TStructTypeBuilder<ThisType, void, tags>; \
                                                                              \
	static Rift::Refl::StructType* GetType()                                  \
	{                                                                         \
		return Rift::GetType<ThisType>();                                     \
	}                                                                         \
	void SerializeReflection(Rift::Archive& ar)                               \
	{                                                                         \
		__ReflSerializeProperty(ar, Rift::Refl::MetaCounter<0>{});            \
	}                                                                         \
	BASESTRUCT(type)


/** Defines an struct */
#define __STRUCT_NO_TAGS(type, parent) __STRUCT_TAGS(type, parent, Rift::ReflectionTags::None)
#define __STRUCT_TAGS(type, parent, tags)                                      \
public:                                                                        \
	using ThisType    = type;                                                  \
	using Super       = parent;                                                \
	using BuilderType = Rift::Refl::TStructTypeBuilder<ThisType, Super, tags>; \
                                                                               \
	static Rift::Refl::StructType* GetType()                                   \
	{                                                                          \
		return Rift::GetType<ThisType>();                                      \
	}                                                                          \
	void SerializeReflection(Rift::Archive& ar)                                \
	{                                                                          \
		Super::SerializeReflection(ar);                                        \
		__ReflSerializeProperty(ar, Rift::Refl::MetaCounter<0>{});             \
	}                                                                          \
	BASESTRUCT(type)


#define BASESTRUCT(type)                                                                    \
public:                                                                                     \
	static Rift::Refl::StructType* InitType()                                               \
	{                                                                                       \
		BuilderType builder{Rift::Name{TX(#type)}};                                         \
		builder.onBuild = [](auto& builder) {                                               \
			__ReflReflectProperty(builder, Rift::Refl::MetaCounter<0>{});                   \
		};                                                                                  \
		builder.Initialize();                                                               \
		return builder.GetType();                                                           \
	}                                                                                       \
                                                                                            \
private:                                                                                    \
	static constexpr Rift::Refl::MetaCounter<0> __refl_Counter(Rift::Refl::MetaCounter<0>); \
	template <Rift::u32 N>                                                                  \
	static void __ReflReflectProperty(BuilderType&, Rift::Refl::MetaCounter<N>)             \
	{}                                                                                      \
	template <Rift::u32 N>                                                                  \
	void __ReflSerializeProperty(Rift::Archive&, Rift::Refl::MetaCounter<N>)                \
	{}                                                                                      \
                                                                                            \
public:


#define __PROPERTY_NO_TAGS(type, name) __PROPERTY_TAGS(type, name, Rift::ReflectionTags::None)
#define __PROPERTY_TAGS(type, name, tags) __PROPERTY_IMPL(type, name, CAT(__refl_id_, name), tags)
#define __PROPERTY_IMPL(type, name, id_name, inTags)                                          \
	static constexpr Rift::u32 id_name =                                                      \
	    decltype(__refl_Counter(Rift::Refl::MetaCounter<255>{}))::value;                      \
	static constexpr Rift::Refl::MetaCounter<(id_name) + 1> __refl_Counter(                   \
	    Rift::Refl::MetaCounter<(id_name) + 1>);                                              \
                                                                                              \
	static void __ReflReflectProperty(BuilderType& builder, Rift::Refl::MetaCounter<id_name>) \
	{                                                                                         \
		static constexpr Rift::ReflectionTags tags =                                          \
		    Rift::ReflectionTagsInitializer<inTags>::value;                                   \
		builder.AddProperty<type, tags>(TX(#name), [](void* instance) {                       \
			return &static_cast<ThisType*>(instance)->name;                                   \
		});                                                                                   \
                                                                                              \
		/* Registry next property if any */                                                   \
		__ReflReflectProperty(builder, Rift::Refl::MetaCounter<(id_name) + 1>{});             \
	};                                                                                        \
                                                                                              \
	void __ReflSerializeProperty(Rift::Archive& ar, Rift::Refl::MetaCounter<id_name>)         \
	{                                                                                         \
		static constexpr Rift::ReflectionTags tags =                                          \
		    Rift::ReflectionTagsInitializer<inTags>::value;                                   \
                                                                                              \
		if constexpr (!(tags & Transient))                                                    \
		{ /* Don't serialize property if Transient */                                         \
			ar(#name, name);                                                                  \
		}                                                                                     \
		/* Serialize next property if any */                                                  \
		__ReflSerializeProperty(ar, Rift::Refl::MetaCounter<(id_name) + 1>{});                \
	};


#define __GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define GET_4TH_ARG(tuple) __GET_4TH_ARG tuple

#define TYPE_INVALID(...) static_assert(false, "Invalid type macro. Missing first parameters");
#define TYPE_CHOOSER(TYPE_NO_TAGS, TYPE_TAGS, ...) \
	GET_4TH_ARG((__VA_ARGS__, TYPE_TAGS, TYPE_NO_TAGS, TYPE_INVALID))
