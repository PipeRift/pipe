// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Serialization/SerializationFwd.h"


namespace p
{
	class StructType;

	struct PIPE_API BaseStruct
	{
		void SerializeReflection(p::ReadWriter& ct) {}
		// StructType* GetType() const;
	};
}    // namespace p
