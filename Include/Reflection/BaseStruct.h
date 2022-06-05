// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Serialization/ContextsFwd.h"


namespace pipe::refl
{
	class StructType;

	struct CORE_API BaseStruct
	{
		void SerializeReflection(pipe::Serl::CommonContext& ct) {}
		// refl::StructType* GetType() const;
	};
}    // namespace pipe::refl

namespace pipe
{
	using namespace pipe::refl;
}
