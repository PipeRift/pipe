// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Memory/MonoLinearArena.h"

#include "Pipe/Memory/Block.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
	void* MonoLinearArena::Alloc(sizet size, sizet align)
	{
		u8* const allocEnd = (u8*)insert + size + GetAlignmentPadding(insert, align);
		// Not enough space in current block?
		if (allocEnd <= block.End()) [[likely]]
		{
			insert = allocEnd;
			++count;
			return allocEnd - size;    // Fast-path
		}

		// Allocation doesn't fit. Allocate in parent arena
		return GetParentArena().Alloc(size, align);
	}

	void MonoLinearArena::Free(void* ptr, sizet size)
	{
		if (ptr >= block.data && ptr < block.End()) [[likely]]
		{
			--count;
			if (count <= 0) [[unlikely]]
			{
				Release(true);
			}
		}
		else
		{
			GetParentArena().Free(ptr, size);
		}
	}

	void MonoLinearArena::Release(bool keepIfSelfAllocated)
	{
		insert = block.data;
		count  = 0;
		if (selfAllocated && !keepIfSelfAllocated)
		{
			// Self allcoated block gets freed
			GetParentArena().Free(block.data, block.size);
			block = {};
		}
	}
}    // namespace p
