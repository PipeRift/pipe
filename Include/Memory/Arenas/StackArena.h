// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Math/Math.h"
#include "Memory/Allocators/IArena.h"
#include "Memory/Blocks/HeapBlock.h"


namespace Rift::Memory
{
	/**
	 * LinearAllocator holds memory linearly in a block of memory.
	 * (Sometimes called ZoneAllocator if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	class CORE_API StackArena : public IArena
	{
	public:
		StackArena() {}
		~StackArena() {}
	};
}    // namespace Rift::Memory
