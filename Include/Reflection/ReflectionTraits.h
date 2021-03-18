// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Object/BaseObject.h"
#include "Reflection/Registry/TypeBuilder.h"
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
	inline constexpr bool IsReflected()
	{
		if constexpr (IsArray<T>())
		{
			return IsReflected<typename T::ItemType>();
		}
		return IsStruct<T>() || IsClass<T>() || IsEnum<T>();
	}


	template <typename T>
	struct HasTypeBuilderDef
	{
	private:
		template <typename V>
		static void Impl(decltype(typename V::TypeBuilder(), int()));
		template <typename V>
		static bool Impl(char);

	public:
		static constexpr bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};

	template <typename T>
	constexpr bool HasTypeBuilder()
	{
		return HasTypeBuilderDef<T>::value && Derived<T::TypeBuilder, BaseBuilder, true>
	}

	template <typename T>
	constexpr bool HasType() requires(IsStruct<T>() || IsClass<T>())
	{
		return HasTypeBuilder()
	}

	template <typename T>
	constexpr bool HasType() requires(IsEnum<T>())
	{
		return Refl::TStaticEnumInitializer<T>::reflected;
	}

	template <typename T>
	constexpr bool HasType()
	{
		return HasTypeBuilder();
	}
}    // namespace Rift
