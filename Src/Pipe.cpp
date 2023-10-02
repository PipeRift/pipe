// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Memory/NewDelete.h"
//  Override as first include

#include "Pipe.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Files/Paths.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Reflect/TypeRegistry.h"


namespace p
{
	void Initialize(StringView logPath)
	{
		InitializeMemory();
		TypeRegistry::Initialize();
		InitLog(ToPath(logPath));
	}

	void Shutdown()
	{
		ShutdownLog();
	}
};    // namespace p
