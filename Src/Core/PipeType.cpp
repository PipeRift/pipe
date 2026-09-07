// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeType.h"

#include "PipeReflect.h"


namespace p::details
{
	bool IsTypeIdCompatible(TypeId parentId, TypeId childId)
	{
		return parentId == childId || IsTypeParentOf(parentId, childId);
	}
}    // namespace p::details
