// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Builders/StaticInitializers.h"
#include "Pipe/Reflect/Builders/TypeBuilder.h"
#include "Pipe/Reflect/ReflectionTraits.h"


namespace p
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

	template<typename T, typename BuilderType = TypeBuilder>
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
	constexpr bool HasType() requires(IsArrayView<T>())
	{
		return HasType<typename T::ItemType>();
	}
}    // namespace p
