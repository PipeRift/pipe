// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Platform/Macros.h"
#include "Platform/Platform.h"
#include "Reflection/TClass.h"
#include "Reflection/TStruct.h"
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

	/* template <typename T>
	auto* GetType()
	{
		return T::Type();
	}

	template <typename T>
	auto* GetType(T* instance)
	{
		return instance->GetType();
	}*/
}	 // namespace Rift::Refl

/** Defines a Class */
#define __CLASS_NO_TAGS(type, parent) __CLASS_TAGS(type, parent, Rift::ReflectionTags::None)
#define __CLASS_TAGS(type, parent, tags)                            \
public:                                                             \
	using Super = parent;                                           \
                                                                    \
	virtual void SerializeReflection(Rift::Archive& ar) override    \
	{                                                               \
		Super::SerializeReflection(ar);                             \
		__refl_SerializeProperty(ar, Rift::Refl::MetaCounter<0>{}); \
	}                                                               \
	static void __refl_Registry()                                   \
	{                                                               \
		StaticType()->__Registry<Super>(Rift::Name{TX(#type)});     \
		TYPETAGS(type, tags)                                        \
	}                                                               \
	BASECLASS(type)


/** Defines a class with no parent */
#define ORPHAN_CLASS(type, tags)                                                 \
public:                                                                          \
	virtual void SerializeReflection(Rift::Archive& ar)                          \
	{                                                                            \
		__refl_SerializeProperty(ar, Rift::Refl::MetaCounter<0>{});              \
	}                                                                            \
	static void __refl_Registry()                                                \
	{                                                                            \
		static_assert(std::is_convertible<__refl_type, Rift::BaseObject>::value, \
			"Type does not inherit Object!");                                    \
		StaticType()->__Registry(Rift::Name{TX(#type)});                         \
		TYPETAGS(type, tags)                                                     \
	}                                                                            \
	BASECLASS(type)

#define BASECLASS(type)                                                                     \
private:                                                                                    \
	using __refl_type = type;                                                               \
	friend Rift::Refl::Type;                                                                \
	friend Rift::Refl::TClass<__refl_type>;                                                 \
                                                                                            \
public:                                                                                     \
	static Rift::Refl::TClass<__refl_type>* StaticType()                                    \
	{                                                                                       \
		return Rift::Refl::TClass<__refl_type>::GetStatic();                                \
	}                                                                                       \
                                                                                            \
	static inline void __refl_RegistryProperties()                                          \
	{                                                                                       \
		__refl_RegistryProperty(Rift::Refl::MetaCounter<0>{});                              \
	}                                                                                       \
                                                                                            \
private:                                                                                    \
	static constexpr Rift::Refl::MetaCounter<0> __refl_Counter(Rift::Refl::MetaCounter<0>); \
	template <Rift::u32 N>                                                                  \
	static void __refl_RegistryProperty(Rift::Refl::MetaCounter<N>)                         \
	{}                                                                                      \
	template <Rift::u32 N>                                                                  \
	void __refl_SerializeProperty(Rift::Archive&, Rift::Refl::MetaCounter<N>)               \
	{}


/** Defines an struct */
#define __STRUCT_NO_TAGS(type, parent) __STRUCT_TAGS(type, parent, Rift::ReflectionTags::None)
#define __STRUCT_TAGS(type, parent, tags)                                    \
public:                                                                      \
	using Super = parent;                                                    \
                                                                             \
	virtual Rift::Refl::Struct* GetType() const override                     \
	{                                                                        \
		return StaticType();                                                 \
	}                                                                        \
	virtual inline void SerializeReflection(Rift::Archive& ar) override      \
	{                                                                        \
		Super::SerializeReflection(ar);                                      \
		__refl_SerializeProperty(ar, Rift::Refl::MetaCounter<0>{});          \
	}                                                                        \
	static void __refl_Registry()                                            \
	{                                                                        \
		static_assert(std::is_convertible<__refl_type, Rift::Struct>::value, \
			"Type does not inherit Struct!");                                \
		StaticType()->__Registry<Super>(Rift::Name{TX(#type)});              \
		TYPETAGS(type, tags)                                                 \
	}                                                                        \
	BASESTRUCT(type)


/** Defines an struct with no parent */
#define ORPHAN_STRUCT(type, tags)                                   \
public:                                                             \
	virtual inline void SerializeReflection(Rift::Archive& ar)      \
	{                                                               \
		__refl_SerializeProperty(ar, Rift::Refl::MetaCounter<0>{}); \
	}                                                               \
	static void __refl_Registry()                                   \
	{                                                               \
		StaticType()->__Registry(Rift::Name{TX(#type)});            \
		TYPETAGS(type, tags)                                        \
	}                                                               \
	BASESTRUCT(type)


#define BASESTRUCT(type)                                                                    \
private:                                                                                    \
	using __refl_type = type;                                                               \
	friend Rift::Refl::Type;                                                                \
	friend Rift::Refl::TStruct<__refl_type>;                                                \
                                                                                            \
	static constexpr Rift::Refl::MetaCounter<0> __refl_Counter(Rift::Refl::MetaCounter<0>); \
	template <Rift::u32 N>                                                                  \
	static void __refl_RegistryProperty(Rift::Refl::MetaCounter<N>)                         \
	{}                                                                                      \
	template <Rift::u32 N>                                                                  \
	void __refl_SerializeProperty(Rift::Archive&, Rift::Refl::MetaCounter<N>)               \
	{}                                                                                      \
                                                                                            \
public:                                                                                     \
	static Rift::Refl::TStruct<__refl_type>* StaticType()                                   \
	{                                                                                       \
		return Rift::Refl::TStruct<__refl_type>::GetStatic();                               \
	}                                                                                       \
                                                                                            \
	static inline void __refl_RegistryProperties()                                          \
	{                                                                                       \
		__refl_RegistryProperty(Rift::Refl::MetaCounter<0>{});                              \
	}


#define TYPETAGS(type, inTags)                                                            \
	constexpr Rift::ReflectionTags tags = Rift::ReflectionTagsInitializer<inTags>::value; \
	static_assert(!(tags & DetailsEdit), "Only properties can use DetailsEdit");          \
	static_assert(!(tags & DetailsView), "Only properties can use DetailsView");          \
	StaticType()->__RegistryTags(tags);


#define __PROPERTY_NO_TAGS(type, name) __PROPERTY_TAGS(type, name, Rift::ReflectionTags::None)
#define __PROPERTY_TAGS(type, name, tags) __PROPERTY_IMPL(type, name, CAT(__refl_id_, name), tags)
#define __PROPERTY_IMPL(type, name, id_name, inTags)                                          \
	static constexpr Rift::u32 id_name =                                                      \
		decltype(__refl_Counter(Rift::Refl::MetaCounter<255>{}))::value;                      \
	static constexpr Rift::Refl::MetaCounter<(id_name) + 1> __refl_Counter(                   \
		Rift::Refl::MetaCounter<(id_name) + 1>);                                              \
                                                                                              \
	static void __refl_RegistryProperty(Rift::Refl::MetaCounter<id_name>)                     \
	{                                                                                         \
		static_assert(                                                                        \
			Rift::IsReflectableType<type>(), "" #type " is not a valid reflectable type.");   \
                                                                                              \
		constexpr Rift::ReflectionTags tags = Rift::ReflectionTagsInitializer<inTags>::value; \
		static_assert(!(tags & Abstract), "Properties can't be Abstract");                    \
                                                                                              \
		StaticType()->__RegistryProperty<type>(                                               \
			TX(#name),                                                                        \
			[](void* instance) {                                                              \
				return &static_cast<__refl_type*>(instance)->name;                            \
			},                                                                                \
			tags);                                                                            \
                                                                                              \
		/* Registry next property if any */                                                   \
		__refl_RegistryProperty(Rift::Refl::MetaCounter<(id_name) + 1>{});                    \
	};                                                                                        \
                                                                                              \
	void __refl_SerializeProperty(Rift::Archive& ar, Rift::Refl::MetaCounter<id_name>)        \
	{                                                                                         \
		constexpr Rift::ReflectionTags tags = Rift::ReflectionTagsInitializer<inTags>::value; \
                                                                                              \
		if constexpr (!(tags & Transient))                                                    \
		{ /* Don't serialize property if Transient */                                         \
			ar(#name, name);                                                                  \
		}                                                                                     \
		/* Serialize next property if any */                                                  \
		__refl_SerializeProperty(ar, Rift::Refl::MetaCounter<(id_name) + 1>{});               \
	};


#define __GET_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define GET_4TH_ARG(tuple) __GET_4TH_ARG tuple

#define TYPE_INVALID(...) static_assert(false, "Invalid type macro. Missing first parameters");
#define TYPE_CHOOSER(TYPE_NO_TAGS, TYPE_TAGS, ...) \
	GET_4TH_ARG((__VA_ARGS__, TYPE_TAGS, TYPE_NO_TAGS, TYPE_INVALID))


#define CLASS(...) TYPE_CHOOSER(__CLASS_NO_TAGS, __CLASS_TAGS, __VA_ARGS__)(__VA_ARGS__)
#define STRUCT(...) TYPE_CHOOSER(__STRUCT_NO_TAGS, __STRUCT_TAGS, __VA_ARGS__)(__VA_ARGS__)
#define PROP(...) TYPE_CHOOSER(__PROPERTY_NO_TAGS, __PROPERTY_TAGS, __VA_ARGS__)(__VA_ARGS__)
