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
#include "Serialization/Contexts.h"
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

		template <typename PropertyType, ReflectionTags propTags>
		void AddProperty(Name name, TFunction<PropertyType*(void*)>&& access)
		{
			static_assert(!(propTags & Abstract), "Properties can't be Abstract");

			Type* const valueType = TTypeInstance<PropertyType>::InitType();

			auto& registry = ReflectionRegistry::Get();
			auto* const ptr =
			    registry.AddProperty<Property>(GetType(), valueType, name, access, propTags);

			GetType()->properties.Insert(name, ptr);
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

			GetType()->onCreate = []() {
				if constexpr (IsSame<T, BaseObject>)
				{
					return nullptr;    // Can't create instances of BaseObject
				}
				return new T();
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


#define CLASS(type, ...)                                                     \
	TYPE_CHOOSER(CLASS_HEADER_NO_TAGS, CLASS_HEADER_TAGS, type, __VA_ARGS__) \
	(type, __VA_ARGS__) CLASS_BODY(type, {})

#define STRUCT(type, ...)                                                      \
	TYPE_CHOOSER(STRUCT_HEADER_NO_TAGS, STRUCT_HEADER_TAGS, type, __VA_ARGS__) \
	(type, __VA_ARGS__) STRUCT_BODY(type, {})

#define PROP(...) TYPE_CHOOSER(PROPERTY_NO_TAGS, PROPERTY_TAGS, __VA_ARGS__)(__VA_ARGS__)


/** Defines a Class */
#define CLASS_HEADER_NO_TAGS(type, parent) \
	CLASS_HEADER_TAGS(type, parent, Rift::ReflectionTags::None)
#define CLASS_HEADER_TAGS(type, parent, tags)                                 \
public:                                                                       \
	using ThisType    = type;                                                 \
	using Super       = parent;                                               \
	using BuilderType = Rift::Refl::TClassTypeBuilder<ThisType, Super, tags>; \
                                                                              \
	Rift::Refl::ClassType* GetClass() const override                          \
	{                                                                         \
		return Rift::GetType<ThisType>();                                     \
	}                                                                         \
	void SerializeReflection(Rift::Serl::CommonContext& ct) override          \
	{                                                                         \
		Super::SerializeReflection(ct);                                       \
		__ReflSerializeProperty(ct, Rift::Refl::MetaCounter<0>{});            \
	}


#define CLASS_BODY(type, buildCode)                                                         \
public:                                                                                     \
	static Rift::Refl::ClassType* InitType()                                                \
	{                                                                                       \
		BuilderType builder{Rift::Name{TX(#type)}};                                         \
		builder.onBuild = [](auto& builder) {                                               \
			__ReflReflectProperty(builder, Rift::Refl::MetaCounter<0>{});                   \
			{                                                                               \
				buildCode                                                                   \
			}                                                                               \
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
	void __ReflSerializeProperty(Rift::Serl::CommonContext&, Rift::Refl::MetaCounter<N>)    \
	{}

/** Defines an struct */
#define STRUCT_HEADER_NO_TAGS(type, parent) \
	STRUCT_HEADER_TAGS(type, parent, Rift::ReflectionTags::None)
#define STRUCT_HEADER_TAGS(type, parent, tags)                                 \
public:                                                                        \
	using Super       = parent;                                                \
	using ThisType    = type;                                                  \
	using BuilderType = Rift::Refl::TStructTypeBuilder<ThisType, Super, tags>; \
                                                                               \
	static Rift::Refl::StructType* GetType()                                   \
	{                                                                          \
		return Rift::GetType<ThisType>();                                      \
	}                                                                          \
	void SerializeReflection(Rift::Serl::CommonContext& ct)                    \
	{                                                                          \
		Super::SerializeReflection(ct);                                        \
		__ReflSerializeProperty(ct, Rift::Refl::MetaCounter<0>{});             \
	}


#define STRUCT_BODY(type, buildCode)                                                        \
public:                                                                                     \
	static Rift::Refl::StructType* InitType()                                               \
	{                                                                                       \
		BuilderType builder{Rift::Name{TX(#type)}};                                         \
		builder.onBuild = [](auto& builder) {                                               \
			__ReflReflectProperty(builder, Rift::Refl::MetaCounter<0>{});                   \
			{                                                                               \
				buildCode                                                                   \
			}                                                                               \
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
	void __ReflSerializeProperty(Rift::Serl::CommonContext&, Rift::Refl::MetaCounter<N>)    \
	{}                                                                                      \
                                                                                            \
public:

#define SET_TYPE_BUILDER(builder) \
public:                           \
	using BuilderType = builder;


#define PROPERTY_NO_TAGS(type, name) PROPERTY_TAGS(type, name, Rift::ReflectionTags::None)
#define PROPERTY_TAGS(type, name, tags) __PROPERTY_IMPL(type, name, CAT(__refl_id_, name), tags)
#define __PROPERTY_IMPL(type, name, id_name, inTags)                                              \
	static constexpr Rift::u32 id_name =                                                          \
	    decltype(__refl_Counter(Rift::Refl::MetaCounter<255>{}))::value;                          \
	static constexpr Rift::Refl::MetaCounter<(id_name) + 1> __refl_Counter(                       \
	    Rift::Refl::MetaCounter<(id_name) + 1>);                                                  \
                                                                                                  \
	static void __ReflReflectProperty(BuilderType& builder, Rift::Refl::MetaCounter<id_name>)     \
	{                                                                                             \
		static_assert(HasType<type>(), "Type is not reflected");                                  \
		static constexpr Rift::ReflectionTags tags =                                              \
		    Rift::ReflectionTagsInitializer<inTags>::value;                                       \
		builder.AddProperty<type, tags>(TX(#name), [](void* instance) {                           \
			return &static_cast<ThisType*>(instance)->name;                                       \
		});                                                                                       \
                                                                                                  \
		/* Registry next property if any */                                                       \
		__ReflReflectProperty(builder, Rift::Refl::MetaCounter<(id_name) + 1>{});                 \
	};                                                                                            \
                                                                                                  \
	void __ReflSerializeProperty(Rift::Serl::CommonContext& ct, Rift::Refl::MetaCounter<id_name>) \
	{                                                                                             \
		static constexpr Rift::ReflectionTags tags =                                              \
		    Rift::ReflectionTagsInitializer<inTags>::value;                                       \
                                                                                                  \
		if constexpr (!(tags & Transient))                                                        \
		{ /* Don't serialize property if Transient */                                             \
			ct.Next(#name, name);                                                                 \
		}                                                                                         \
		/* Serialize next property if any */                                                      \
		__ReflSerializeProperty(ct, Rift::Refl::MetaCounter<(id_name) + 1>{});                    \
	};


#define __GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define GET_4TH_ARG(tuple) __GET_4TH_ARG tuple

#define TYPE_INVALID(...) static_assert(false, "Invalid type macro. Missing first parameters");
#define TYPE_CHOOSER(TYPE_NO_TAGS, TYPE_TAGS, ...) \
	GET_4TH_ARG((__VA_ARGS__, TYPE_TAGS, TYPE_NO_TAGS, TYPE_INVALID))
