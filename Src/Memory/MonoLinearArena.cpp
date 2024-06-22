// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Memory/MonoLinearArena.h"

#include "Pipe/Memory/Block.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
	void* MonoLinearArena::Alloc(sizet size, sizet align)
	{
		u8* const allocEnd = (u8*)insert + size + GetAlignmentPadding(insert, align);
		void* ptr;
		// Not enough space in current block?
		if (allocEnd <= block.End()) [[likely]]
		{
			insert = allocEnd;
			++count;
			ptr = allocEnd - size;    // Fast-path
		}
		else
		{
			// Allocation doesn't fit. Allocate in parent arena
			ptr = GetParentArena().Alloc(size, align);
		}
		stats.Add(ptr, size);
		return ptr;
	}

	void MonoLinearArena::Free(void* ptr, sizet size)
	{
		stats.Remove(ptr, size);
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
		stats.Release();
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
