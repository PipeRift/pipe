// Copyright 2015-2022 Piperift - All rights reserved

#include "ECS/Pool.h"


namespace p::ecs
{
	i32 GetSmallestPool(TSpan<const Pool*> pools)
	{
		sizet minSize = std::numeric_limits<sizet>::max();
		i32 minIndex  = NO_INDEX;
		for (i32 i = 0; i < pools.Size(); ++i)
		{
			const i32 size = pools[i]->Size();
			if (size < minSize)
			{
				minSize  = size;
				minIndex = i;
			}
		}
		return minIndex;
	}
}    // namespace p::ecs
