// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Events/Function.h"
#include "Reflection/Registry/Registry.h"
#include "Reflection/Registry/StaticInitializers.h"
#include "Reflection/Registry/TypeBuilder.h"
#include "Reflection/Static/EnumType.h"
#include "Reflection/TypeId.h"
#include "Strings/Name.h"
#include "TypeTraits.h"


namespace Rift::Refl
{
	template <typename T>
	struct TStaticEnumInitializer
	{
		static constexpr bool reflected = false;
		static const TFunction<EnumType*()> onInit;
	};
	template <typename T>
	inline const TFunction<EnumType*()> TStaticEnumInitializer<T>::onInit{};

}    // namespace Rift::Refl
