// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Pipe.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Memory/NewDelete.h"
#include "Pipe/Reflect/TypeRegistry.h"


P_DO_OVERRIDE_NEW_DELETE


namespace p
{
	void Initialize(Path logPath)
	{
		TypeRegistry::Initialize();
		InitLog(logPath);
	}

	void Shutdown()
	{
		ShutdownLog();
	}
};    // namespace p
