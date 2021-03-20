// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

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


	template <typename T>
	Refl::StructType* GetType() requires(IsStruct<T>())
	{
		static_assert(HasType<T>(), "T is not reflected and doesnt have a type.");
		return static_cast<Refl::StructType*>(Refl::InternalGetType<T>());
	}

	template <typename T>
	Refl::ClassType* GetType() requires(IsClass<T>())
	{
		static_assert(HasType<T>(), "T is not reflected and doesnt have a type.");
		return static_cast<Refl::ClassType*>(Refl::InternalGetType<T>());
	}

	template <typename T>
	Refl::EnumType* GetType() requires(IsEnum<T>())
	{
		static_assert(HasType<T>(), "T is not reflected and doesnt have a type.");
		return static_cast<Refl::EnumType*>(Refl::InternalGetType<T>());
	}

	template <typename T>
	Refl::Type* GetType()
	{
		static_assert(HasType<T>(), "T is not reflected and doesnt have a type.");
		return Refl::InternalGetType<T>();
	}
}    // namespace Rift
