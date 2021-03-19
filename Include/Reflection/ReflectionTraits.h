// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Containers/Array.h"
#include "Object/BaseObject.h"
#include "Reflection/Registry/TypeBuilder.h"
#include "TypeTraits.h"



namespace Rift
{
	struct Struct;

	template <typename T>
	inline constexpr bool IsEnum()
	{
		return std::is_enum<T>::value;
	}

	template <typename T>
	inline constexpr bool IsStruct()
	{
		return Derived<T, Struct>;    // && IsDefined<struct TTypeInstance<T>>();
	}

	template <typename T>
	inline constexpr bool IsClass()
	{
		return Derived<T, BaseObject, false>;    // && IsDefined<struct TTypeInstance<T>>();
	}

	template <typename T>
	inline constexpr bool IsArray()
	{
		// Check if we are dealing with a TArray
		if constexpr (HasItemType<T>::value)
		{
			return IsSame<TArray<typename T::ItemType>, T>;
		}
		return false;
	}
}    // namespace Rift
