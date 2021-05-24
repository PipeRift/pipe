// Copyright 2015-2021 Piperift - All rights reserved

#include "Context.h"


namespace Rift
{
	TOwnPtr<Context> globalInstance{};


	void ShutdownContext()
	{
		if (globalInstance)
		{
			globalInstance.Delete();
		}
	}

	TOwnPtr<Context>& InternalGetContext()
	{
		return globalInstance;
	}
}
