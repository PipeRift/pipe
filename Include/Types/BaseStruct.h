// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Serialization/ContextsFwd.h"


namespace Pipe
{
	namespace Refl
	{
		class StructType;
	}

	struct CORE_API BaseStruct
	{
		void SerializeReflection(Pipe::Serl::CommonContext& ct) {}
		// Refl::StructType* GetType() const;
	};
}    // namespace Pipe
