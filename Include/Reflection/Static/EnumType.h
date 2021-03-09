// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Reflection/Static/BaseType.h"
#include "Strings/Name.h"

#include <magic_enum.hpp>


namespace Rift::Refl
{
	/** Smallest reflection type that contains all basic class or struct data */
	class CORE_API EnumType : public BaseType
	{
		template <typename T>
		friend struct TEnumTypeBuilder;

	protected:
		Name name;


	public:
		EnumType() = default;
	};
}    // namespace Rift::Refl
