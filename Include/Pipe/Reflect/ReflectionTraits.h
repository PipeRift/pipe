// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Map.h"
#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Reflect/BaseClass.h"


namespace p
{
	template<typename Type>
	struct IArray;

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

	template<typename T, typename = int>
	struct HasInlineCapacityMember : std::false_type
	{};
	template<typename T>
	struct HasInlineCapacityMember<T, decltype((void)T::inlineCapacity, 0)> : std::true_type
	{};

	template<typename T>
	inline constexpr bool IsArray()
	{
		// Check if we are dealing with a TArray
		if constexpr (HasItemTypeMember<T>::value && HasInlineCapacityMember<T>::value)
		{
			return Derived<T, TInlineArray<typename T::ItemType, T::inlineCapacity>>;
		}
		return false;
	}

	template<typename T>
	inline constexpr bool IsArrayView()
	{
		// Check if we are dealing with a TArray
		if constexpr (HasItemTypeMember<T>::value)
		{
			return Derived<T, IArray<typename T::ItemType>>;
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
