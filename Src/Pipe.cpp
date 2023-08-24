// Copyright 2015-2023 Piperift - All rights reserved

// #include "Pipe/Memory/NewDelete.h"
//  Override as first include

#include "Pipe/Pipe.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Reflect/TypeRegistry.h"


namespace p
{
	void Initialize(Path logPath)
	{
		TypeRegistry::Initialize();
		// InitLog(logPath);
	}

	void Shutdown()
	{
		ShutdownLog();
	}
};    // namespace p
