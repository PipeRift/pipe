// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Object/BaseObject.h"
#include "Strings/Name.h"
#include "TypeTraits.h"

#include <type_traits>


namespace Rift
{
	struct Struct;

	template <typename T>
	class TAssetPtr;

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
	inline constexpr bool IsEnum()
	{
		return std::is_enum<T>::value;
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

	template <typename T>
	inline constexpr bool IsAsset()
	{
		// Check if we are dealing with a TAssetPtr
		if constexpr (HasItemType<T>::value)
		{
			return IsSame<TAssetPtr<typename T::ItemType>, T>;
		}
		return false;
	}

	template <typename T>
	inline constexpr bool IsReflected()
	{
		if constexpr (IsArray<T>())
		{
			return IsReflected<typename T::ItemType>();
		}
		return IsAsset<T>() || IsStruct<T>() || IsClass<T>() || IsEnum<T>();
	}

	template <typename T>
	inline Name GetReflectedName()
	{
		if constexpr (IsArray<T>())
		{
			if constexpr (IsReflected<typename T::ItemType>())
			{
				// TArray<Itemtype> name
				return {CString::Format(
				    TX("TArray<{}>"), GetReflectedName<typename T::ItemType>().ToString())};
			}
			return TX("TArray<Invalid>");
		}
		else if constexpr (IsAsset<T>())
		{
			// TAssetPtr<Itemtype> name
			return {CString::Format(
			    TX("TAssetPtr<{}>"), GetReflectedName<typename T::ItemType>().ToString())};
		}
		else if constexpr (IsStruct<T>() || IsClass<T>())
		{
			return GetType<T>()->GetName();
		}
		return TX("Invalid");
	}
}    // namespace Rift


#define DECLARE_REFLECTED_TYPE(type)           \
	template <>                                \
	inline constexpr bool IsReflected<type>()  \
	{                                          \
		return true;                           \
	}                                          \
	template <>                                \
	inline Name GetReflectedName<type>()       \
	{                                          \
		static const Name typeName{TX(#type)}; \
		return typeName;                       \
	}
