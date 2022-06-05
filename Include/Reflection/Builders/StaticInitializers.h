// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Core/Function.h"


namespace pipe
{
	namespace refl
	{
		class EnumType;
		class NativeType;


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
	}    // namespace refl


	template<typename T>
	inline constexpr bool IsReflectedEnum()
	{
		return refl::TStaticEnumInitializer<T>::enabled;
	}

	template<typename T>
	inline constexpr bool IsReflectedNative()
	{
		return refl::TStaticNativeInitializer<T>::enabled;
	}
}    // namespace pipe
