// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Reflection/ReflectionTraits.h"
#include "Reflection/TypeName.h"
#include "Strings/FixedString.h"


OVERRIDE_TYPE_NAME(Pipe::u8)
OVERRIDE_TYPE_NAME(Pipe::u16)
OVERRIDE_TYPE_NAME(Pipe::u32)
OVERRIDE_TYPE_NAME(Pipe::u64)
OVERRIDE_TYPE_NAME(Pipe::i8)
OVERRIDE_TYPE_NAME(Pipe::i16)
OVERRIDE_TYPE_NAME(Pipe::i32)
OVERRIDE_TYPE_NAME(Pipe::i64)
OVERRIDE_TYPE_NAME(Pipe::TChar)
OVERRIDE_TYPE_NAME(Pipe::StringView)

namespace Pipe::Refl
{
	// Contains an static fixed string with the name of a TArray<T>
	template<typename ItemType, bool includeNamespaces>
	struct StaticArrayName
	{
		static constexpr auto preffix        = TFixedString("TArray<");
		static constexpr auto suffix         = TFixedString(">");
		static constexpr StringView itemName = GetFullTypeName<ItemType>(includeNamespaces);
		static constexpr TFixedString<itemName.size()> fixedItemName{itemName};

		static constexpr auto name = preffix + fixedItemName + suffix;
	};

	// Contains an static fixed string with the name of a TMap<Key, Value>
	template<typename KeyType, typename ValueType, bool includeNamespaces>
	struct StaticMapName
	{
		static constexpr auto preffix         = TFixedString("TMap<");
		static constexpr auto separator       = TFixedString(", ");
		static constexpr auto suffix          = TFixedString(">");
		static constexpr StringView keyName   = GetFullTypeName<KeyType>(includeNamespaces);
		static constexpr StringView valueName = GetFullTypeName<ValueType>(includeNamespaces);
		static constexpr TFixedString<keyName.size()> fixedKeyName{keyName};
		static constexpr TFixedString<valueName.size()> fixedValueName{valueName};

		static constexpr auto name = preffix + fixedKeyName + separator + fixedValueName + suffix;
	};


	template<typename T>
	consteval StringView GetFullTypeName(bool includeNamespaces = true) requires(IsArray<T>())
	{
		if (includeNamespaces)
		{
			return StaticArrayName<typename T::ItemType, true>::name;
		}
		return StaticArrayName<typename T::ItemType, false>::name;
	}

	template<typename T>
	inline consteval StringView GetTypeName(bool includeNamespaces = true) requires(IsArray<T>())
	{
		return "TArray";
	}

	template<typename T>
	consteval StringView GetFullTypeName(bool includeNamespaces = true) requires(IsMap<T>())
	{
		if (includeNamespaces)
		{
			return StaticMapName<typename T::KeyType, typename T::ValueType, true>::name;
		}
		return StaticMapName<typename T::KeyType, typename T::ValueType, false>::name;
	}

	template<typename T>
	consteval StringView GetTypeName(bool includeNamespaces = true) requires(IsMap<T>())
	{
		return "TMap";
	}

}    // namespace Pipe::Refl


namespace Pipe
{
	using namespace Pipe::Refl;
}
