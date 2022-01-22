// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/Type.h"
#include "Strings/Name.h"


namespace Rift::Refl
{
	/** Smallest reflection type that contains all basic class or struct data */
	class NativeType : public Type
	{
		template<typename T>
		friend struct TNativeTypeBuilder;

	protected:
		Name name;

	public:
		CORE_API NativeType() = default;
	};
}    // namespace Rift::Refl
