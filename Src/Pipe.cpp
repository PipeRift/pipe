// Copyright 2015-2024 Piperift - All rights reserved

// #include "Pipe/Memory/NewDelete.h"
//   Override as first include

#include "Pipe.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Files/Paths.h"
#include "Pipe/Memory/Alloc.h"
#include "PipeReflect.h"


namespace p
{
	void Initialize(StringView logPath)
	{
		InitializeMemory();
		InitializeReflect();
		InitLog(logPath);
	}

	void Shutdown()
	{
		ShutdownLog();
	}
};    // namespace p
