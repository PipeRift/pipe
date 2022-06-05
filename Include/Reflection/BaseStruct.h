// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Serialization/ContextsFwd.h"


namespace p::refl
{
	class StructType;

	struct CORE_API BaseStruct
	{
		void SerializeReflection(p::serl::CommonContext& ct) {}
		// refl::StructType* GetType() const;
	};
}    // namespace p::refl

namespace p
{
	using namespace p::refl;
}
