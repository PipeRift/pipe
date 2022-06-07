// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Function.h"


namespace p
{
	class EnumType;
	class NativeType;


	namespace reflection
	{
		template<typename T>
		struct TStaticEnumInitializer
		{
			static constexpr bool enabled = false;
			static const TFunction<EnumType*()> onInit;
		};

		template<typename T>
		inline const TFunction<EnumType*()> TStaticEnumInitializer<T>::onInit{};


		template<typename T>
		struct TStaticNativeInitializer
		{
			static constexpr bool enabled = false;
			static const TFunction<NativeType*()> onInit;
		};

		template<typename T>
		inline const TFunction<NativeType*()> TStaticNativeInitializer<T>::onInit{};
	}    // namespace reflection


	template<typename T>
	inline constexpr bool IsReflectedEnum()
	{
		return reflection::TStaticEnumInitializer<T>::enabled;
	}

	template<typename T>
	inline constexpr bool IsReflectedNative()
	{
		return reflection::TStaticNativeInitializer<T>::enabled;
	}
}    // namespace p
