// Copyright 2015-2024 Piperift - All rights reserved

// #include "Pipe/Memory/NewDelete.h"
//   Override as first include

#include "Pipe.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Files/Paths.h"
#include "PipeMemory.h"
#include "PipeReflect.h"


namespace p
{
	void Initialize(Logger* logger)
	{
		InitializeMemory();
		InitializeReflect();
		InitLog(logger);
	}

	void Shutdown()
	{
		ShutdownLog();
	}
};    // namespace p
