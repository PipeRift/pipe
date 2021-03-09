// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift::Refl
{
	/** Smallest reflection type */
	class CORE_API BaseType
	{
	protected:
		BaseType() = default;

	public:
		BaseType(const BaseType&) = delete;
		BaseType& operator=(const BaseType&) = delete;
	};
}    // namespace Rift::Refl
