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
	protected:
		Name name;

	public:
		NativeType() = default;
	};

#define REFLECT_NATIVE_TYPE(type)    // TODO
}    // namespace Rift::Refl
