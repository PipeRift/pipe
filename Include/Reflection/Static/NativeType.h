// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/Type.h"
#include "Strings/Name.h"


namespace Rift::Refl
{
	/** Smallest reflection type that contains all basic class or struct data */
	class CORE_API NativeType : public Type
	{
		template <typename T>
		friend struct TNativeTypeBuilder;

	protected:
		Name name;

	public:
		NativeType() = default;
	};
}    // namespace Rift::Refl
