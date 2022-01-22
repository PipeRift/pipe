// Copyright 2015-2022 Piperift - All rights reserved

#include "Context.h"


namespace Rift
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
	}
}    // namespace Rift
