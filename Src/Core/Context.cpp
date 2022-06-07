// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Core/Context.h"


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
	}
}    // namespace p
