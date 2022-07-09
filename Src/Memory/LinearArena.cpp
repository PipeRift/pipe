// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/LinearArena.h"

#include "Pipe/Memory/Memory.h"


namespace p
{
	void* LinearArena::Alloc(sizet size)
	{
		if (usedBlockSize + size > activeBlock.size)
		{
			if (!allowGrowing)
			{
				return nullptr;
			}
			// Grow same size as previous block, but make sure its enough space
			Grow(math::Max(activeBlock.size, size));
		}

		void* const ptr = (u8*)(activeBlock.data) + usedBlockSize;
		usedBlockSize += size;
		return ptr;
	}

	void* LinearArena::Alloc(sizet size, sizet alignment)
	{
		if (alignment == 0)
		{
			return Alloc(size);    // Allocate without alignment
		}

		void* currentPtr    = (u8*)(activeBlock.data) + usedBlockSize;
		const sizet padding = GetAlignmentPadding(currentPtr, alignment);

		// Not enough space in current block?
		if (usedBlockSize + size + padding > activeBlock.size)
		{
			if (!allowGrowing)
			{
				return nullptr;
			}
			// Grow same size as previous block, but make sure its enough space
			// NOTE: We use minimum size + alignment to make sure a
			// non aligned Grow allocates enough memory
			Grow(math::Max(activeBlock.size, size + alignment), alignment);

			// Try again with new block
			return Alloc(size, alignment);
		}

		usedBlockSize += size + padding;
		return (u8*)(currentPtr) + padding;
	}

	void LinearArena::Reset()
	{
		usedBlockSize = 0;
		p::Free(GetParentArena(), activeBlock.data, activeBlock.size);
		activeBlock = {};
		for (Memory::Block& block : discardedBlocks)
		{
			p::Free(GetParentArena(), block.data, block.size);
			block.data = nullptr;
		}
		discardedBlocks.Empty();
	}

	void LinearArena::Grow(sizet size, sizet /*align*/)
	{
		if (size > 0)    // Don't reserve an empty block
		{
			// Push last block for destructor deletion
			discardedBlocks.Add(Move(activeBlock));

			// TODO: Support aligned blocks
			activeBlock.data = p::Alloc(GetParentArena(), size);
			activeBlock.size = size;
			usedBlockSize    = 0;
		}
	}
}    // namespace p
