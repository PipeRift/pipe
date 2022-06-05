// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/OwnPtr.h"
#include "Reflection/Builders/TypeInstance.h"
#include "Reflection/HasType.h"
#include "Reflection/ReflectionTraits.h"


namespace p
{
	namespace refl
	{
		class StructType;
		class ClassType;
		class EnumType;
	}    // namespace refl


	template<typename T>
	refl::StructType* GetType() requires(IsStruct<T>())
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		return static_cast<refl::StructType*>(refl::TTypeInstance<T>::GetType());
	}

	template<typename T>
	refl::ClassType* GetType() requires(IsClass<T>())
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		return static_cast<refl::ClassType*>(refl::TTypeInstance<T>::GetType());
	}

	template<typename T>
	refl::EnumType* GetType() requires(IsReflectedEnum<T>())
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		return static_cast<refl::EnumType*>(refl::TTypeInstance<T>::GetType());
	}

	template<typename T>
	refl::Type* GetType()
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		return refl::TTypeInstance<T>::GetType();
	}

	/**
	 * Finds a type by name, where template T is the base.
	 * Template type is included in the search.
	 * @param name of the type to find. If none, type of T will be returned
	 * @returns the type found, if any
	 */
	template<typename T>
	refl::Type* FindType(Name name)
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
	refl::Type* GetType(T& instance)
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
	refl::Type* GetType(TOwnPtr<T>& instance)
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
	refl::Type* GetType(TPtr<T>& instance)
	{
		static_assert(HasType<T>(), "T is not reflected and doesn't have a type.");
		if (instance && IsClass<T>())
		{
			return instance->GetType();
		}
		return GetType<T>();
	}
}    // namespace p

namespace p
{
	using namespace p::refl;
}
