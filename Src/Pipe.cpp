// Copyright 2015-2026 Piperift. All Rights Reserved.

//#include "PipeNewDelete.h"
// New/Delete must be first include

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
		ShutdownReflect();
		ShutdownMemory();
	}
};    // namespace p
