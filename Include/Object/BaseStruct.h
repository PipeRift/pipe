// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift
{
	namespace Refl
	{
		class Struct;
	}

	struct CORE_API BaseStruct
	{
		BaseStruct() = default;
		virtual ~BaseStruct() {}

		Refl::Struct* GetType() const;
	};
}	 // namespace Rift