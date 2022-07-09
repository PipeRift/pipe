// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/Arena.h"

#include "Pipe/Memory/HeapArena.h"


namespace p
{
	ChildArena::ChildArena(Arena* inParent) : parent{inParent}
	{
		if (!parent)
		{
			parent = GetCurrentArena();
			if (parent == this) [[unlikely]]
			{
				parent = &GetHeapArena();
			}
		}
	}
}    // namespace p
