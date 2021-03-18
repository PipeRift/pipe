// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/ReflectionTraits.h"


namespace Rift
{
	namespace Refl
	{
		class StructType;
		class ClassType;
		class EnumType;
		class Type;
	}    // namespace Refl


	template <typename T>
	Refl::StructType* GetType() requires(IsStruct<T>())
	{
		return static_cast<Refl::StructType*>(__GetType<T>());
	}

	template <typename T>
	Refl::ClassType* GetType() requires(IsClass<T>())
	{
		return static_cast<Refl::ClassType*>(__GetType<T>());
	}

	template <typename T>
	Refl::EnumType* GetType() requires(IsEnum<T>())
	{
		return static_cast<Refl::EnumType*>(__GetType<T>());
	}

	template <typename T>
	Refl::Type* GetType()
	{
		return __GetType<T>();
	}

	template <typename T>
	Refl::Type* __GetType()
	{
		static_assert(HasType<T>(), "T is not reflected and doesnt have a type.");
		return Refl::TTypeInstance<T>::instance;
	}
}    // namespace Rift
