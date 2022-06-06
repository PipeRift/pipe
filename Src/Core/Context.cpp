// Copyright 2015-2022 Piperift - All rights reserved

#include "Core/Context.h"


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
