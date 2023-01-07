// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/TypeTraits.h"
#include "Pipe/Reflect/Type.h"


namespace p
{
	/** Smallest reflection type that contains all basic class or struct data */
	class NativeType : public Type
	{
		template<typename T>
		friend struct TNativeTypeBuilder;

	public:
		static constexpr TypeCategory typeCategory = TypeCategory::Native;


	public:
		PIPE_API NativeType() : Type(typeCategory) {}
	};
}    // namespace p
