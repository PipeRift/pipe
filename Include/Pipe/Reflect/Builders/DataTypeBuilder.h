// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Function.h"
#include "Pipe/Core/Macros.h"
#include "Pipe/Core/Tag.h"
#include "Pipe/Reflect/ArrayProperty.h"
#include "Pipe/Reflect/Builders/TypeBuilder.h"
#include "Pipe/Reflect/ClassType.h"
#include "Pipe/Reflect/Property.h"
#include "Pipe/Reflect/StructType.h"
#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Reflect/TypeName.h"
#include "Pipe/Reflect/TypeRegistry.h"
#include "Pipe/Serialize/Reader.h"
#include "Pipe/Serialize/Writer.h"
#include "PipeArrays.h"



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
		void AddProperty(Tag name, Property::AccessFunc* access, PropFlags propFlags)
		{
			auto& registry = TypeRegistry::Get();
			Type* type;
			Property* property;
			if constexpr (IsArray<U>())
			{
				propFlags       = propFlags | Prop_Array;
				type            = TCompiledTypeRegister<typename U::ItemType>::InitType();
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
					{
						auto& itemRef = *static_cast<typename U::ItemType*>(item);
						if constexpr (IsCopyAssignable<typename U::ItemType>)
						{
							static_cast<U*>(data)->Add(itemRef);
						}
						else
						{
							static_cast<U*>(data)->Add(Move(itemRef));
						}
					}
					else
						static_cast<U*>(data)->Add({});
				};
				arrayProp->removeItem = [](void* data, i32 index) {
					static_cast<U*>(data)->RemoveAt(index);
				};
				arrayProp->clear = [](void* data) {
					static_cast<U*>(data)->Clear();
				};

				property = arrayProp;
			}
			else
			{
				type     = TCompiledTypeRegister<U>::InitType();
				property = registry.AddProperty<Property>();
			}
			property->owner       = Cast<DataType>(GetType());
			property->type        = type;
			property->name        = name;
			property->access      = access;
			property->flags       = propFlags;
			property->displayName = Strings::ToSentenceCase(name.AsString());
			property->owner->properties.Add(property);
		}

	protected:
		Type* CreateType() override
		{
			TType* newType;
			if constexpr (hasParent)
			{
				// Parent gets initialized before anything else
				auto* parent = static_cast<DataType*>(TCompiledTypeRegister<Parent>::InitType());
				Check(parent);

				newType = &TypeRegistry::Get().AddType<TType>(GetId());
				parent->children.Add(newType);
				newType->parent = parent;
			}
			else
			{
				newType = &TypeRegistry::Get().AddType<TType>(GetId());
			}
			newType->size  = sizeof(T);
			newType->id    = id;
			newType->name  = name;
			newType->flags = flags;
			newType->read  = [](Reader& r, void* instance) {
                if constexpr (Readable<T>)
                {
                    r.Serialize(*static_cast<T*>(instance));
                }
			};
			newType->write = [](Writer& w, void* instance) {
				if constexpr (Writable<T>)
				{
					w.Serialize(*static_cast<T*>(instance));
				}
			};

			initializedType = newType;
			return newType;
		}
	};

	template<typename T, typename Parent, TypeFlags flags = Type_NoFlag, typename TType = ClassType>
	struct TClassTypeBuilder : public TDataTypeBuilder<T, Parent, flags, TType>
	{
		static_assert(IsClass<T>(), "Type does not inherit Class!");
		using Super = TDataTypeBuilder<T, Parent, flags, TType>;
		using Super::GetType;

	public:
		TClassTypeBuilder() = default;

	protected:
		Type* CreateType() override
		{
			auto* type                   = Super::CreateType();
			Cast<ClassType>(type)->onNew = [](Arena& arena) -> BaseClass* {
				if constexpr (!IsAbstract<T> && !IsSame<T, BaseClass>)
				{
					return new (p::Alloc<T>(arena)) T();
				}
				return nullptr;    // Can't create instances of abstract classes or
				                   // BaseClass
			};
			return type;
		}
	};


	template<typename T, typename Parent, TypeFlags flags = Type_NoFlag,
	    typename TType = StructType>
	struct TStructTypeBuilder : public TDataTypeBuilder<T, Parent, flags, TType>
	{
		static_assert(IsStruct<T>(), "Type does not inherit Struct!");
		static_assert(!(flags & Class_Abstract), "Only classes can use Class_Abstract");

		using Super = TDataTypeBuilder<T, Parent, flags, TType>;
		using Super::GetType;
		using BuildFunc = TFunction<void(TStructTypeBuilder* builder)>;

	public:
		TStructTypeBuilder() = default;
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
#define P_CLASS_HEADER_NO_FLAGS(type, parent) P_CLASS_HEADER_FLAGS(type, parent, p::Type_NoFlag)
#define P_CLASS_HEADER_FLAGS(type, parent, flags)                                \
private:                                                                         \
	using ThisType = type;                                                       \
	static inline p::TCompiledTypeRegister<ThisType> typeRegister{};             \
                                                                                 \
public:                                                                          \
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


/** Defines an Struct */
#define P_STRUCT_HEADER_NO_FLAGS(type, parent) P_STRUCT_HEADER_FLAGS(type, parent, p::Type_NoFlag)
#define P_STRUCT_HEADER_FLAGS(type, parent, flags)                                \
private:                                                                          \
	using ThisType = type;                                                        \
	static inline p::TCompiledTypeRegister<ThisType> typeRegister{};              \
                                                                                  \
public:                                                                           \
	using Super       = parent;                                                   \
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


#define P_REFLECTION_BODY(buildCode)                                       \
public:                                                                    \
	static p::Type* InitType()                                             \
	{                                                                      \
		BuilderType builder{};                                             \
		if (builder.BeginBuild())                                          \
		{                                                                  \
			__ReflReflectProperty(builder, p::MetaCounter<0>{});           \
			{buildCode} builder.EndBuild();                                \
		}                                                                  \
		return builder.GetType();                                          \
	}                                                                      \
                                                                           \
	void SerializeReflection(p::ReadWriter& ct)                            \
	{                                                                      \
		if constexpr (!(GetStaticFlags() & p::Type_NotSerialized))         \
		{                                                                  \
			Super::SerializeReflection(ct);                                \
			__ReflSerializeProperty(ct, p::MetaCounter<0>{});              \
		}                                                                  \
	}                                                                      \
                                                                           \
private:                                                                   \
	static constexpr p::MetaCounter<0> __refl_Counter(p::MetaCounter<0>);  \
	template<p::u32 index>                                                 \
	static void __ReflReflectProperty(BuilderType&, p::MetaCounter<index>) \
	{}                                                                     \
	template<p::u32 index>                                                 \
	void __ReflSerializeProperty(p::ReadWriter&, p::MetaCounter<index>)    \
	{}                                                                     \
                                                                           \
public:


#define P_PROPERTY_NO_FLAGS(name) P_PROPERTY_FLAGS(name, p::Prop_NoFlag)
#define P_PROPERTY_FLAGS(name, flags) __P_PROPERTY_IMPL(name, CAT(__refl_id_, name), flags)
#define __P_PROPERTY_IMPL(name, id_name, flags)                                                   \
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
		__ReflReflectProperty(builder, p::MetaCounter<id_name + 1>{});                            \
	};                                                                                            \
                                                                                                  \
	void __ReflSerializeProperty(p::ReadWriter& ct, p::MetaCounter<id_name>)                      \
	{                                                                                             \
		if constexpr (!(p::InitPropFlags(flags) & p::Prop_NotSerialized))                         \
		{ /* Don't serialize property if Transient */                                             \
			ct.Next(#name, name);                                                                 \
		}                                                                                         \
		/* Serialize next property if any */                                                      \
		__ReflSerializeProperty(ct, p::MetaCounter<id_name + 1>{});                               \
	};


#define P_REFL_INTERNAL_GET_3RD_ARG(arg1, arg2, arg3, ...) arg3
#define P_REFL_GET_3RD_ARG(tuple) P_REFL_INTERNAL_GET_3RD_ARG tuple

#define P_REFL_INTERNAL_GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define P_REFL_GET_4TH_ARG(tuple) P_REFL_INTERNAL_GET_4TH_ARG tuple

#define P_TYPE_INVALID(...) static_assert(false, "Invalid type macro. Missing first parameters");

#define P_TYPE_CHOOSER(TYPE_NO_FLAGS, TYPE_FLAGS, type, parent, ...) \
	P_REFL_GET_4TH_ARG((type, parent, __VA_ARGS__, TYPE_FLAGS, TYPE_NO_FLAGS, TYPE_INVALID))

#define P_PROP_CHOOSER(TYPE_NO_FLAGS, TYPE_FLAGS, name, ...) \
	P_REFL_GET_3RD_ARG((name, __VA_ARGS__, TYPE_FLAGS, TYPE_NO_FLAGS, TYPE_INVALID))


#define P_CLASS(type, parent, ...)                                                           \
	P_TYPE_CHOOSER(P_CLASS_HEADER_NO_FLAGS, P_CLASS_HEADER_FLAGS, type, parent, __VA_ARGS__) \
	(type, parent, __VA_ARGS__) P_REFLECTION_BODY({})

#define P_STRUCT(type, parent, ...)                                                            \
	P_TYPE_CHOOSER(P_STRUCT_HEADER_NO_FLAGS, P_STRUCT_HEADER_FLAGS, type, parent, __VA_ARGS__) \
	(type, parent, __VA_ARGS__) P_REFLECTION_BODY({})

#define P_PROP(name, ...) \
	P_PROP_CHOOSER(P_PROPERTY_NO_FLAGS, P_PROPERTY_FLAGS, name, __VA_ARGS__)(name, __VA_ARGS__)
