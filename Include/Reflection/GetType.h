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
		class Type;

		template <typename T>
		Refl::Type* __GetType()
		{
			static_assert(HasType<T>(), "T is not reflected and doesnt have a type.");
			return TTypeInstance<T>::instance;
		}
	}    // namespace Refl


	template <typename T>
	Refl::StructType* GetType() requires(IsStruct<T>())
	{
		return static_cast<Refl::StructType*>(Refl::__GetType<T>());
	}

	template <typename T>
	Refl::ClassType* GetType() requires(IsClass<T>())
	{
		return static_cast<Refl::ClassType*>(Refl::__GetType<T>());
	}

	template <typename T>
	Refl::EnumType* GetType() requires(IsEnum<T>())
	{
		return static_cast<Refl::EnumType*>(Refl::__GetType<T>());
	}

	template <typename T>
	Refl::Type* GetType()
	{
		return Refl::__GetType<T>();
	}
}    // namespace Rift
