// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "Object/BaseObject.h"
#include "Reflection/Registry/TypeBuilder.h"
#include "TypeTraits.h"


namespace Rift
{
	struct Struct;

	template<typename T>
	inline constexpr bool IsStruct()
	{
		return Derived<T, Struct>;    // && IsDefined<struct TTypeInstance<T>>();
	}

	template<typename T>
	inline constexpr bool IsClass()
	{
		return Derived<T, BaseObject, false>;    // && IsDefined<struct TTypeInstance<T>>();
	}

	template<typename T>
	inline constexpr bool IsArray()
	{
		// Check if we are dealing with a TArray
		if constexpr (HasItemType<T>::value)
		{
			return IsSame<TArray<typename T::ItemType>, T>;
		}
		return false;
	}

	template<typename T>
	inline constexpr bool IsMap()
	{
		// Check if we are dealing with a TAssetPtr
		if constexpr (HasKeyType<T>::value && HasValueType<T>::value)
		{
			return IsSame<
			    TMap<typename T::KeyType, typename T::ValueType, typename T::AllocatorType>, T>;
		}
		return false;
	}
}    // namespace Rift
