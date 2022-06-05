// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Type.h"


namespace pipe::refl
{
	/** Smallest reflection type that contains all basic class or struct data */
	class NativeType : public Type
	{
		template<typename T>
		friend struct TNativeTypeBuilder;

	public:
		CORE_API NativeType() : Type(TypeCategory::Native) {}
	};
}    // namespace pipe::refl

namespace pipe
{
	using namespace pipe::refl;
}
