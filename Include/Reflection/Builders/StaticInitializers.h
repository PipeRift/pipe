// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Core/Function.h"


namespace Pipe
{
	namespace Refl
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
	}    // namespace Refl


	template<typename T>
	inline constexpr bool IsReflectedEnum()
	{
		return Refl::TStaticEnumInitializer<T>::enabled;
	}

	template<typename T>
	inline constexpr bool IsReflectedNative()
	{
		return Refl::TStaticNativeInitializer<T>::enabled;
	}
}    // namespace Pipe
