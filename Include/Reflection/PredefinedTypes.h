// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Reflection/ReflectionTraits.h"
#include "Reflection/TypeName.h"
#include "Strings/FixedString.h"


namespace Rift
{
	OVERRIDE_TYPE_NAME(u8)
	OVERRIDE_TYPE_NAME(u16)
	OVERRIDE_TYPE_NAME(u32)
	OVERRIDE_TYPE_NAME(u64)
	OVERRIDE_TYPE_NAME(i8)
	OVERRIDE_TYPE_NAME(i16)
	OVERRIDE_TYPE_NAME(i32)
	OVERRIDE_TYPE_NAME(i64)
	OVERRIDE_TYPE_NAME(TChar)
	OVERRIDE_TYPE_NAME(StringView)


	// Contains an static fixed string with the name of a TArray<T>
	template <typename ItemType>
	struct StaticArrayName
	{
		static constexpr auto preffix        = TFixedString("TArray<");
		static constexpr auto suffix         = TFixedString(">");
		static constexpr StringView itemName = GetTypeName<ItemType>();
		static constexpr TFixedString<itemName.size()> fixedItemName{itemName};

		static constexpr auto name = preffix + fixedItemName + suffix;
	};

	// Contains an static fixed string with the name of a TAssetPtr<T>
	template <typename KeyType, typename ValueType>
	struct StaticMapName
	{
		static constexpr auto preffix         = TFixedString("TMap<");
		static constexpr auto separator       = TFixedString(", ");
		static constexpr auto suffix          = TFixedString(">");
		static constexpr StringView keyName   = GetTypeName<KeyType>();
		static constexpr StringView valueName = GetTypeName<ValueType>();
		static constexpr TFixedString<keyName.size()> fixedKeyName{keyName};
		static constexpr TFixedString<valueName.size()> fixedValueName{valueName};

		static constexpr auto name = preffix + fixedKeyName + separator + fixedValueName + suffix;
	};


	template <typename T>
	constexpr StringView GetFullTypeName() requires(IsArray<T>())
	{
		return StaticArrayName<typename T::ItemType>::name;
	}

	template <typename T>
	inline constexpr StringView GetTypeName() requires(IsArray<T>())
	{
		return "TArray";
	}

	template <typename T>
	constexpr StringView GetFullTypeName() requires(IsMap<T>())
	{
		return StaticMapName<typename T::KeyType, typename T::ValueType>::name;
	}

	template <typename T>
	inline constexpr StringView GetTypeName() requires(IsMap<T>())
	{
		return "TMap";
	}
}    // namespace Rift
