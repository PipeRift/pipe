// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"


namespace Rift::Refl
{
	class EnumType;
	class NativeType;


	template <typename T>
	struct TStaticEnumInitializer
	{
		static constexpr bool enabled = false;
		static const TFunction<EnumType*()> onInit;
	};
	template <typename T>
	inline const TFunction<EnumType*()> TStaticEnumInitializer<T>::onInit{};


	template <typename T>
	struct TStaticNativeInitializer
	{
		static constexpr bool enabled = false;
		static const TFunction<NativeType*()> onInit;
	};
	template <typename T>
	inline const TFunction<NativeType*()> TStaticNativeInitializer<T>::onInit{};
}    // namespace Rift::Refl
