// Copyright 2015-2021 Piperift - All rights reserved

#include "Object/Object.h"

#include "Context.h"


namespace Rift
{
	TPtr<Context> Object::GetContext() const
	{
		return Context::Get();
	}
}    // namespace Rift
