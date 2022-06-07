// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Export.h"
#include "Pipe/Serialize/SerializationFwd.h"


namespace p
{
	class StructType;

	struct PIPE_API BaseStruct
	{
		void SerializeReflection(p::ReadWriter& ct) {}
		// StructType* GetType() const;
	};
}    // namespace p
