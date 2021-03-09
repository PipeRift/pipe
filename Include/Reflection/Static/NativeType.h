// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Containers/Map.h"
#include "CoreTypes.h"
#include "Reflection/ReflectionTags.h"
#include "Reflection/Static/BaseType.h"
#include "Reflection/Static/Property.h"
#include "Strings/String.h"


namespace Rift::Refl
{
	/** Smallest reflection type that contains all basic class or struct data */
	class CORE_API NativeType : public BaseType
	{
	protected:
		Name name;

	public:
		NativeType() = default;
	};
}    // namespace Rift::Refl
