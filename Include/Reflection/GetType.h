// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/OwnPtr.h"
#include "Reflection/HasType.h"
#include "Reflection/ReflectionTraits.h"
#include "Reflection/Registry/TypeInstance.h"


namespace Rift
{
	namespace Refl
	{
		class StructType;
		class ClassType;
		class EnumType;
	}    // namespace Refl


	template<typename T>
	Refl::StructType* GetType() requires(IsStruct<T>())
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		return static_cast<Refl::StructType*>(Refl::InternalGetType<T>());
	}

	template<typename T>
	Refl::ClassType* GetType() requires(IsClass<T>())
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		return static_cast<Refl::ClassType*>(Refl::InternalGetType<T>());
	}

	template<typename T>
	Refl::EnumType* GetType() requires(IsReflectedEnum<T>())
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		return static_cast<Refl::EnumType*>(Refl::InternalGetType<T>());
	}

	template<typename T>
	Refl::Type* GetType()
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		return Refl::InternalGetType<T>();
	}

	/**
	 * Finds a type by name, where template T is the base.
	 * Template type is included in the search.
	 * @param name of the type to find. If none, type of T will be returned
	 * @returns the type found, if any
	 */
	template<typename T>
	Refl::Type* FindType(Name name)
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");

		if constexpr (IsStruct<T>() || IsClass<T>())
		{
			auto* type = GetType<T>();
			if (!type)
			{
				type = type->FindChild(name);
			}
			return type;
		}
		return GetType<T>();
	}

	/**
	 * Get the type of an instance.
	 * Classes will obtain their type from inheritance.
	 */
	template<typename T>
	Refl::Type* GetType(T& instance)
	{
		if constexpr (IsClass<T>())
		{
			return instance.GetType();
		}
		return GetType<T>();
	}

	/**
	 * Get the type from an OwnPtr.
	 * Classes will obtain their type from inheritance.
	 */
	template<typename T>
	Refl::Type* GetType(TOwnPtr<T>& instance)
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		if (instance && IsClass<T>())
		{
			return instance->GetType();
		}
		return GetType<T>();
	}

	/**
	 * Get the type from an OwnPtr.
	 * Classes will obtain their type from inheritance.
	 */
	template<typename T>
	Refl::Type* GetType(TPtr<T>& instance)
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		if (instance && IsClass<T>())
		{
			return instance->GetType();
		}
		return GetType<T>();
	}
}    // namespace Rift
