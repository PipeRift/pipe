// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Platform/Macros.h"
#include "Platform/Platform.h"
#include "Reflection/Static/ReflectionRegistry.h"
#include "Reflection/Static/TClass.h"
#include "Reflection/Static/TStruct.h"
#include "ReflectionTags.h"


namespace Rift::Refl
{
	/**
	 * There are 3 types of reflected data structures:
	 *
	 * - POD: No inheritance, base classes or Garbage Collection. Only contains
	 * data.
	 * - STRUCT: Inheritance but no logic or garbage collection. Used for
	 * Components.
	 * - CLASS: Inheritance, Logic and Garbage Collection. Used as Objects.
	 */


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

/** Defines a Class */
#define __CLASS_NO_TAGS(type, parent) __CLASS_TAGS(type, parent, Rift::ReflectionTags::None)
#define __CLASS_TAGS(type, parent, tags)                                               \
public:                                                                                \
	using ThisType = type;                                                             \
	using Super    = parent;                                                           \
	using TypeBuilder =                                                                \
	    Rift::Refl::TTypeBuilder<ThisType, Super, Rift::Refl::TClass<ThisType>, tags>; \
                                                                                       \
	Rift::Refl::Class* GetType() const override                                        \
	{                                                                                  \
		return StaticType();                                                           \
	}                                                                                  \
	void SerializeReflection(Rift::Archive& ar) override                               \
	{                                                                                  \
		Super::SerializeReflection(ar);                                                \
		__ReflSerializeProperty(ar, Rift::Refl::MetaCounter<0>{});                     \
	}                                                                                  \
	BASECLASS(type)


/** Defines a class with no parent */
#define ORPHAN_CLASS(type, tags)                                                      \
public:                                                                               \
	using ThisType = type;                                                            \
	using TypeBuilder =                                                               \
	    Rift::Refl::TTypeBuilder<ThisType, void, Rift::Refl::TClass<ThisType>, tags>; \
                                                                                      \
	virtual Rift::Refl::Class* GetType() const                                        \
	{                                                                                 \
		return StaticType();                                                          \
	}                                                                                 \
	virtual void SerializeReflection(Rift::Archive& ar)                               \
	{                                                                                 \
		__ReflSerializeProperty(ar, Rift::Refl::MetaCounter<0>{});                    \
	}                                                                                 \
	BASECLASS(type)


#define BASECLASS(type)                                                                     \
public:                                                                                     \
	static Rift::Refl::TClass<ThisType>* StaticType()                                       \
	{                                                                                       \
		return Rift::Refl::TClass<ThisType>::GetStatic();                                   \
	}                                                                                       \
	static Rift::Refl::TClass<ThisType>* InitType()                                         \
	{                                                                                       \
		static TypeBuilder builder{                                                         \
		    TX(__FILE__), __LINE__, Rift::Name{TX(#type)}, [](auto& builder) {              \
			    __ReflBuildProperty(builder, Rift::Refl::MetaCounter<0>{});                 \
		    }};                                                                             \
		return builder.GetType();                                                           \
	}                                                                                       \
                                                                                            \
private:                                                                                    \
	static constexpr Rift::Refl::MetaCounter<0> __refl_Counter(Rift::Refl::MetaCounter<0>); \
	template <Rift::u32 N>                                                                  \
	static void __ReflBuildProperty(TypeBuilder&, Rift::Refl::MetaCounter<N>)               \
	{}                                                                                      \
	template <Rift::u32 N>                                                                  \
	void __ReflSerializeProperty(Rift::Archive&, Rift::Refl::MetaCounter<N>)                \
	{}


/** Defines an struct */
#define __STRUCT_NO_TAGS(type, parent) __STRUCT_TAGS(type, parent, Rift::ReflectionTags::None)
#define __STRUCT_TAGS(type, parent, tags)                                               \
public:                                                                                 \
	using ThisType = type;                                                              \
	using Super    = parent;                                                            \
	using TypeBuilder =                                                                 \
	    Rift::Refl::TTypeBuilder<ThisType, Super, Rift::Refl::TStruct<ThisType>, tags>; \
                                                                                        \
	Rift::Refl::Struct* GetType() const override                                        \
	{                                                                                   \
		return StaticType();                                                            \
	}                                                                                   \
	void SerializeReflection(Rift::Archive& ar) override                                \
	{                                                                                   \
		Super::SerializeReflection(ar);                                                 \
		__ReflSerializeProperty(ar, Rift::Refl::MetaCounter<0>{});                      \
	}                                                                                   \
	BASESTRUCT(type)


/** Defines an struct with no parent */
#define ORPHAN_STRUCT(type, tags)                                                      \
public:                                                                                \
	using ThisType = type;                                                             \
	using TypeBuilder =                                                                \
	    Rift::Refl::TTypeBuilder<ThisType, void, Rift::Refl::TStruct<ThisType>, tags>; \
                                                                                       \
	virtual Rift::Refl::Struct* GetType() const                                        \
	{                                                                                  \
		return StaticType();                                                           \
	}                                                                                  \
	virtual void SerializeReflection(Rift::Archive& ar)                                \
	{                                                                                  \
		__ReflSerializeProperty(ar, Rift::Refl::MetaCounter<0>{});                     \
	}                                                                                  \
	BASESTRUCT(type)


#define BASESTRUCT(type)                                                                    \
public:                                                                                     \
	static Rift::Refl::TStruct<ThisType>* StaticType()                                      \
	{                                                                                       \
		return Rift::Refl::TStruct<ThisType>::GetStatic();                                  \
	}                                                                                       \
	static Rift::Refl::TStruct<ThisType>* InitType()                                        \
	{                                                                                       \
		static TypeBuilder builder{                                                         \
		    TX(__FILE__), __LINE__, Rift::Name{TX(#type)}, [](auto& builder) {              \
			    __ReflBuildProperty(builder, Rift::Refl::MetaCounter<0>{});                 \
		    }};                                                                             \
		return builder.GetType();                                                           \
	}                                                                                       \
                                                                                            \
private:                                                                                    \
	static constexpr Rift::Refl::MetaCounter<0> __refl_Counter(Rift::Refl::MetaCounter<0>); \
	template <Rift::u32 N>                                                                  \
	static void __ReflBuildProperty(TypeBuilder&, Rift::Refl::MetaCounter<N>)               \
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
	static void __ReflBuildProperty(TypeBuilder& builder, Rift::Refl::MetaCounter<id_name>)   \
	{                                                                                         \
		constexpr Rift::ReflectionTags tags = Rift::ReflectionTagsInitializer<inTags>::value; \
		builder.AddProperty<type, tags>(TX(#name), [](void* instance) {                       \
			return &static_cast<ThisType*>(instance)->name;                                   \
		});                                                                                   \
                                                                                              \
		/* Registry next property if any */                                                   \
		__ReflBuildProperty(builder, Rift::Refl::MetaCounter<(id_name) + 1>{});               \
	};                                                                                        \
                                                                                              \
	void __ReflSerializeProperty(Rift::Archive& ar, Rift::Refl::MetaCounter<id_name>)         \
	{                                                                                         \
		constexpr Rift::ReflectionTags tags = Rift::ReflectionTagsInitializer<inTags>::value; \
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


#define CLASS(...) TYPE_CHOOSER(__CLASS_NO_TAGS, __CLASS_TAGS, __VA_ARGS__)(__VA_ARGS__)
#define STRUCT(...) TYPE_CHOOSER(__STRUCT_NO_TAGS, __STRUCT_TAGS, __VA_ARGS__)(__VA_ARGS__)
#define PROP(...) TYPE_CHOOSER(__PROPERTY_NO_TAGS, __PROPERTY_TAGS, __VA_ARGS__)(__VA_ARGS__)
