// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Reflect/Struct.h"


namespace p::ecs
{
	// Mark an entity as not serialized.
	// Will be entirely ignored by the serializer
	struct CNotSerialized : public Struct
	{
		STRUCT(CNotSerialized, Struct)
	};
}    // namespace p::ecs
