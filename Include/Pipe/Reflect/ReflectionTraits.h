// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Map.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Reflect/BaseClass.h"


namespace p
{
	template<typename Type, u32 InlineCapacity>
	struct TInlineArray;

	struct Struct;

	template<typename T>
	inline constexpr bool IsStruct()
	{
		return Derived<T, Struct>;    // && IsDefined<struct TCompiledTypeRegister<T>>();
	}

	template<typename T>
	inline constexpr bool IsClass()
	{
		return Derived<T, BaseClass, false>;    // && IsDefined<struct TCompiledTypeRegister<T>>();
	}

	template<typename T>
	inline constexpr bool IsArray()
	{
		// Check if we are dealing with a TArray
		if constexpr (HasItemTypeMember<T>::value)
		{
			return IsSame<T, TInlineArray<typename T::ItemType, 0>>;
		}
		return false;
	}

	template<typename T>
	inline constexpr bool IsMap()
	{
		// Check if we are dealing with a TAssetPtr
		if constexpr (HasKeyTypeMember<T>::value && HasValueTypeMember<T>::value)
		{
			return IsSame<TMap<typename T::KeyType, typename T::ValueType>, T>;
		}
		return false;
	}
}    // namespace p
