// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Type.h"


namespace p
{
	/** Smallest reflection type that contains all basic class or struct data */
	class NativeType : public Type
	{
		template<typename T>
		friend struct TNativeTypeBuilder;

	public:
		PIPE_API NativeType() : Type(TypeCategory::Native) {}
	};
}    // namespace p
