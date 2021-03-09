// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/ReflectionMacros.h"
#include "Reflection/TypeId.h"


namespace Rift::Refl
{
	template <typename T>
	Struct* GetType() requires(IsStruct<T>())
	{
		if constexpr (IsDefined<struct TTypeInstance<T>>())
		{
			return static_cast<Struct*>(TTypeInstance<T>::instance);
		}
		return nullptr;
	}

	template <typename T>
	Class* GetType() requires(IsClass<T>())
	{
		if constexpr (IsDefined<struct TTypeInstance<T>>())
		{
			return static_cast<Class*>(TTypeInstance<T>::instance);
		}
		return nullptr;
	}

	template <typename T>
	EnumType* GetType() requires(IsEnum<T>())
	{
		if constexpr (IsDefined<struct TTypeInstance<T>>())
		{
			return static_cast<EnumType*>(TTypeInstance<T>::instance);
		}
		return nullptr;
	}
}    // namespace Rift::Refl
