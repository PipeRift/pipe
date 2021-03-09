// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/BaseType.h"
#include "Strings/Name.h"


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
