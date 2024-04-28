// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Export.h"
#include "PipeSerializeFwd.h"


namespace p
{
	class StructType;

	struct PIPE_API BaseStruct
	{
		void SerializeReflection(p::ReadWriter& ct) {}
	};
}    // namespace p
