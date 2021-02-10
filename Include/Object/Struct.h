// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "BaseStruct.h"
#include "Reflection/Reflection.h"
#include "Reflection/Struct.h"



namespace Rift
{
	struct CORE_API Struct : public BaseStruct
	{
		ORPHAN_STRUCT(Struct, ReflectionTags::None)

		virtual Refl::Struct* GetType() const
		{
			return StaticType();
		}
	};
}	 // namespace Rift
