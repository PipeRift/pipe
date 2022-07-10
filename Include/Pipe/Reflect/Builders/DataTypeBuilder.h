// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Function.h"
#include "Pipe/Core/Name.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Reflect/ArrayProperty.h"
#include "Pipe/Reflect/Builders/TypeBuilder.h"
#include "Pipe/Reflect/ClassType.h"
#include "Pipe/Reflect/Property.h"
#include "Pipe/Reflect/Registry.h"
#include "Pipe/Reflect/StructType.h"
#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Serialize/Serialization.h"


namespace p
{
	template<typename T, typename Parent, TypeFlags flags, typename TType>
	struct TDataTypeBuilder : public TypeBuilder
	{
		static constexpr bool hasParent = !std::is_void_v<Parent>;
		static_assert(!hasParent || Derived<T, Parent, false>, "Type must derive from parent.");

	public:
		TDataTypeBuilder() : TypeBuilder(GetTypeId<T>(), GetTypeName<T>(false)) {}

		template<typename U>
		void AddProperty(Name name, Property::AccessFunc* access, PropFlags propFlags)
		{
			auto& registry = ReflectionRegistry::Get();
			Type* type;
			Property* property;
			if constexpr (IsArray<U>())
			{
				propFlags       = propFlags | Prop_Array;
				type            = TTypeInstance<typename U::ItemType>::InitType();
				auto* arrayProp = registry.AddProperty<ArrayProperty>();

				arrayProp->getData = [](void* data) {
					return (void*)static_cast<U*>(data)->Data();
				};
				arrayProp->getSize = [](void* data) {
					return static_cast<U*>(data)->Size();
				};
				arrayProp->getItem = [](void* data, i32 index) {
					return (void*)(static_cast<U*>(data)->Data() + index);
				};
				arrayProp->addItem = [](void* data, void* item) {
					if (item)
						static_cast<U*>(data)->Add(*static_cast<typename U::ItemType*>(item));
					else
						static_cast<U*>(data)->Add({});
				};
				arrayProp->removeItem = [](void* data, i32 index) {
					static_cast<U*>(data)->RemoveAt(index);
				};
				arrayProp->empty = [](void* data) {
					static_cast<U*>(data)->Empty();
				};

				property = arrayProp;
			}
			else
			{
				type     = TTypeInstance<U>::InitType();
				property = registry.AddProperty<Property>();
			}
			property->owner       = GetType();
			property->type        = type;
			property->name        = name;
			property->access      = access;
			property->flags       = propFlags;
			property->displayName = Strings::ToSentenceCase(name.ToString());
			GetType()->properties.Add(property);
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
				auto* parent = static_cast<DataType*>(TTypeInstance<Parent>::InitType());
				Check(parent);

				newType = &ReflectionRegistry::Get().AddType<TType>(GetId());
				parent->children.Add(newType);
				newType->parent = parent;
			}
			else
			{
				newType = &ReflectionRegistry::Get().AddType<TType>(GetId());
			}
			newType->size  = sizeof(T);
			newType->id    = id;
			newType->name  = name;
			newType->flags = flags;

			initializedType = newType;
			return newType;
		}
	};

	template<typename T, typename Parent, TypeFlags flags = Type_NoFlag, typename TType = ClassType>
	struct TClassTypeBuilder : public TDataTypeBuilder<T, Parent, flags, TType>
	{
		static_assert(IsClass<T>(), "Type does not inherit Class!");
		using Super     = TDataTypeBuilder<T, Parent, flags, TType>;
		using BuildFunc = TFunction<void(TClassTypeBuilder& builder)>;
		using Super::GetType;

		BuildFunc onBuild;


	public:
		TClassTypeBuilder() = default;

	protected:
		Type* Build() override
		{
			auto* type = Super::Build();

			GetType()->onNew = [](Arena& arena) -> BaseClass* {
				if constexpr (!IsAbstract<T> && !IsSame<T, BaseClass>)
				{
					return new (p::Alloc<T>(arena)) T();
				}
				return nullptr;    // Can't create instances of abstract classes or
				                   // BaseClass
			};

			if (onBuild)
			{
				onBuild(*this);
			}
			return type;
		}
	};


	template<typename T, typename Parent, TypeFlags flags = Type_NoFlag,
	    typename TType = StructType>
	struct TStructTypeBuilder : public TDataTypeBuilder<T, Parent, flags, TType>
	{
		static_assert(IsStruct<T>(), "Type does not inherit Struct!");
		static_assert(!(flags & Class_Abstract), "Only classes can use Class_Abstract");

		using Super     = TDataTypeBuilder<T, Parent, flags, TType>;
		using BuildFunc = TFunction<void(TStructTypeBuilder& builder)>;
		using Super::GetType;

		BuildFunc onBuild;


	public:
		TStructTypeBuilder() = default;

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
	template<u32 N>
	struct MetaCounter : MetaCounter<N - 1>
	{
		static constexpr u32 value = N;
	};
	template<>
	struct MetaCounter<0>
	{
		static constexpr u32 value = 0;
	};
}    // namespace p


/** Defines a Class */
#define CLASS_HEADER_NO_FLAGS(type, parent) CLASS_HEADER_FLAGS(type, parent, p::Type_NoFlag)
#define CLASS_HEADER_FLAGS(type, parent, flags)                                  \
public:                                                                          \
	using ThisType    = type;                                                    \
	using Super       = parent;                                                  \
	using BuilderType = p::TClassTypeBuilder<ThisType, Super, GetStaticFlags()>; \
                                                                                 \
	static p::ClassType* GetStaticType()                                         \
	{                                                                            \
		return p::GetType<ThisType>();                                           \
	}                                                                            \
	static constexpr p::TypeFlags GetStaticFlags()                               \
	{                                                                            \
		return p::InitTypeFlags(flags);                                          \
	}                                                                            \
	p::ClassType* GetType() const override                                       \
	{                                                                            \
		return p::GetType<ThisType>();                                           \
	}

#define REFLECTION_BODY(buildCode)                                        \
public:                                                                   \
	static p::Type* InitType()                                            \
	{                                                                     \
		BuilderType builder{};                                            \
		builder.onBuild = [](auto& builder) {                             \
			__ReflReflectProperty(builder, p::MetaCounter<0>{});          \
			{                                                             \
				buildCode                                                 \
			}                                                             \
		};                                                                \
		builder.Initialize();                                             \
		return builder.GetType();                                         \
	}                                                                     \
                                                                          \
	void SerializeReflection(p::ReadWriter& ct)                           \
	{                                                                     \
		if constexpr (!(GetStaticFlags() & p::Type_NotSerialized))        \
		{                                                                 \
			Super::SerializeReflection(ct);                               \
			__ReflSerializeProperty(ct, p::MetaCounter<0>{});             \
		}                                                                 \
	}                                                                     \
                                                                          \
private:                                                                  \
	static constexpr p::MetaCounter<0> __refl_Counter(p::MetaCounter<0>); \
	template<p::u32 N>                                                    \
	static void __ReflReflectProperty(BuilderType&, p::MetaCounter<N>)    \
	{}                                                                    \
	template<p::u32 N>                                                    \
	void __ReflSerializeProperty(p::ReadWriter&, p::MetaCounter<N>)       \
	{}                                                                    \
                                                                          \
public:


/** Defines an struct */
#define STRUCT_HEADER_NO_FLAGS(type, parent) STRUCT_HEADER_FLAGS(type, parent, p::Type_NoFlag)
#define STRUCT_HEADER_FLAGS(type, parent, flags)                                  \
public:                                                                           \
	using Super       = parent;                                                   \
	using ThisType    = type;                                                     \
	using BuilderType = p::TStructTypeBuilder<ThisType, Super, GetStaticFlags()>; \
                                                                                  \
	static p::StructType* GetStaticType()                                         \
	{                                                                             \
		return p::GetType<ThisType>();                                            \
	}                                                                             \
	static constexpr p::TypeFlags GetStaticFlags()                                \
	{                                                                             \
		return p::InitTypeFlags(flags);                                           \
	}

#define PROPERTY_NO_FLAGS(name) PROPERTY_FLAGS(name, p::Prop_NoFlag)
#define PROPERTY_FLAGS(name, flags) __PROPERTY_IMPL(name, CAT(__refl_id_, name), flags)
#define __PROPERTY_IMPL(name, id_name, flags)                                                     \
	static constexpr p::u32 id_name = decltype(__refl_Counter(p::MetaCounter<255>{}))::value;     \
	static constexpr p::MetaCounter<(id_name) + 1> __refl_Counter(p::MetaCounter<(id_name) + 1>); \
                                                                                                  \
	static void __ReflReflectProperty(BuilderType& builder, p::MetaCounter<id_name>)              \
	{                                                                                             \
		using PropType = decltype(name);                                                          \
		static_assert(p::HasType<PropType>(), "Type is not reflected");                           \
		builder.AddProperty<PropType>(                                                            \
		    TX(#name),                                                                            \
		    [](void* instance) {                                                                  \
			return (void*)&static_cast<ThisType*>(instance)->name;                                \
		    },                                                                                    \
		    p::InitPropFlags(flags));                                                             \
                                                                                                  \
		/* Registry next property if any */                                                       \
		__ReflReflectProperty(builder, p::MetaCounter<(id_name) + 1>{});                          \
	};                                                                                            \
                                                                                                  \
	void __ReflSerializeProperty(p::ReadWriter& ct, p::MetaCounter<id_name>)                      \
	{                                                                                             \
		if constexpr (!(p::InitPropFlags(flags) & p::Prop_NotSerialized))                         \
		{ /* Don't serialize property if Transient */                                             \
			ct.Next(#name, name);                                                                 \
		}                                                                                         \
		/* Serialize next property if any */                                                      \
		__ReflSerializeProperty(ct, p::MetaCounter<(id_name) + 1>{});                             \
	};


#define REFL_INTERNAL_GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
#define REFL_GET_3RD_ARG(tuple) REFL_INTERNAL_GET_3RD_ARG tuple

#define REFL_INTERNAL_GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define REFL_GET_4TH_ARG(tuple) REFL_INTERNAL_GET_4TH_ARG tuple

#define TYPE_INVALID(...) static_assert(false, "Invalid type macro. Missing first parameters");

#define TYPE_CHOOSER(TYPE_NO_FLAGS, TYPE_FLAGS, type, parent, ...) \
	REFL_GET_4TH_ARG((type, parent, __VA_ARGS__, TYPE_FLAGS, TYPE_NO_FLAGS, TYPE_INVALID))

#define PROP_CHOOSER(TYPE_NO_FLAGS, TYPE_FLAGS, name, ...) \
	REFL_GET_3RD_ARG((name, __VA_ARGS__, TYPE_FLAGS, TYPE_NO_FLAGS, TYPE_INVALID))


#define CLASS(type, parent, ...)                                                       \
	TYPE_CHOOSER(CLASS_HEADER_NO_FLAGS, CLASS_HEADER_FLAGS, type, parent, __VA_ARGS__) \
	(type, parent, __VA_ARGS__) REFLECTION_BODY({})

#define STRUCT(type, parent, ...)                                                        \
	TYPE_CHOOSER(STRUCT_HEADER_NO_FLAGS, STRUCT_HEADER_FLAGS, type, parent, __VA_ARGS__) \
	(type, parent, __VA_ARGS__) REFLECTION_BODY({})

#define PROP(name, ...) \
	PROP_CHOOSER(PROPERTY_NO_FLAGS, PROPERTY_FLAGS, name, __VA_ARGS__)(name, __VA_ARGS__)
