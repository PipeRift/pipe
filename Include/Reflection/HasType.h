// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Builders/StaticInitializers.h"
#include "Reflection/Builders/TypeBuilder.h"
#include "Reflection/ReflectionTraits.h"



namespace Rift
{
	template<typename T>
	struct HasTypeBuilderDefine
	{
	private:
		template<typename V>
		static void Impl(decltype(typename V::BuilderType(), int()));
		template<typename V>
		static bool Impl(char);

	public:
		static constexpr bool value = std::is_void<decltype(Impl<T>(0))>::value;
	};

	template<typename T, typename BuilderType = Refl::TypeBuilder>
	constexpr bool HasTypeBuilder()
	{
		if constexpr (HasTypeBuilderDefine<T>::value)
		{
			return Derived<typename T::BuilderType, BuilderType, true>;
		}
		return false;
	}


	template<typename T>
	constexpr bool HasType()
	{
		return false;
	}

	template<typename T>
	constexpr bool HasType() requires(IsStruct<T>() || IsClass<T>())
	{
		return HasTypeBuilder<T>();
	}

	template<typename T>
	constexpr bool HasType() requires(IsReflectedEnum<T>())
	{
		return true;
	}

	template<typename T>
	constexpr bool HasType() requires(IsReflectedNative<T>())
	{
		return true;
	}

	template<typename T>
	constexpr bool HasType() requires(IsArray<T>())
	{
		return HasType<typename T::ItemType>();
	}
}    // namespace Rift
