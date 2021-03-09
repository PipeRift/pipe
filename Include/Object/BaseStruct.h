// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"


namespace Rift
{
	namespace Refl
	{
		class StructType;
	}

	struct CORE_API BaseStruct
	{
		BaseStruct()  = default;
		~BaseStruct() = default;

		// Refl::StructType* GetType() const;
	};
}    // namespace Rift
