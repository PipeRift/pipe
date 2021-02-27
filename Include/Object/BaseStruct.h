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
		BaseStruct()  = default;
		~BaseStruct() = default;

		// Refl::Struct* GetType() const;
	};
}    // namespace Rift
