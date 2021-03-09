// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/ReflectionMacros.h"
#include "Reflection/TypeId.h"


namespace Rift
{
	template <typename T>
	Refl::StructType* GetType() requires(IsStruct<T>())
	{
		if constexpr (IsDefined<struct Refl::TTypeInstance<T>>())
		{
			return static_cast<Refl::StructType*>(Refl::TTypeInstance<T>::instance);
		}
		return nullptr;
	}

	template <typename T>
	Refl::ClassType* GetType() requires(IsClass<T>())
	{
		if constexpr (IsDefined<struct Refl::TTypeInstance<T>>())
		{
			return static_cast<Refl::ClassType*>(Refl::TTypeInstance<T>::instance);
		}
		return nullptr;
	}

	template <typename T>
	Refl::EnumType* GetType() requires(IsEnum<T>())
	{
		if constexpr (IsDefined<struct Refl::TTypeInstance<T>>())
		{
			return static_cast<Refl::EnumType*>(Refl::TTypeInstance<T>::instance);
		}
		return nullptr;
	}
}    // namespace Rift
