// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Core/Array.h"
#include "Core/Map.h"
#include "Core/TypeTraits.h"
#include "Reflection/BaseClass.h"



namespace p
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
		return Derived<T, BaseClass, false>;    // && IsDefined<struct TTypeInstance<T>>();
	}

	template<typename T>
	inline constexpr bool IsArray()
	{
		// Check if we are dealing with a TArray
		if constexpr (HasItemTypeMember<T>::value)
		{
			return IsSame<TArray<typename T::ItemType>, T>;
		}
		return false;
	}

	template<typename T>
	inline constexpr bool IsMap()
	{
		// Check if we are dealing with a TAssetPtr
		if constexpr (HasKeyTypeMember<T>::value && HasValueTypeMember<T>::value)
		{
			return IsSame<
			    TMap<typename T::KeyType, typename T::ValueType, typename T::AllocatorType>, T>;
		}
		return false;
	}
}    // namespace p
