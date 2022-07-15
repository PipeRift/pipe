// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Core/Context.h"

#include "Pipe/Reflect/Registry.h"


namespace p
{
	TOwnPtr<Context> globalInstance;


	TOwnPtr<Context>& GetContextInstance()
	{
		return globalInstance;
	}

	void ShutdownContext()
	{
		if (globalInstance)
		{
			globalInstance.Delete();
		}

		// Manually reset
		ReflectionRegistry::Get().Reset();
	}
}    // namespace p
