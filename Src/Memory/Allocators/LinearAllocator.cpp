// Copyright 2015-2021 Piperift - All rights reserved

#include "Memory/Allocators/LinearAllocator.h"


namespace Rift::Memory
{
	void* LinearAllocator::Allocate(const sizet size)
	{
		if (usedBlockSize + size > activeBlock.GetSize())
		{
			if (!allowGrowing)
			{
				return nullptr;
			}
			// Grow same size as previous block, but make sure its enough space
			Grow(Math::Max(activeBlock.GetSize(), size));
		}

		void* const ptr = (u8*) (activeBlock.GetData()) + usedBlockSize;
		usedBlockSize += size;
		return ptr;
	}

	void* LinearAllocator::Allocate(const sizet size, const sizet alignment)
	{
		if (alignment == 0)
		{
			return Allocate(size);    // Allocate without alignment
		}

		void* currentPtr    = (u8*) (activeBlock.GetData()) + usedBlockSize;
		const sizet padding = GetAlignmentPadding(currentPtr, alignment);

		// Not enough space in current block?
		if (usedBlockSize + size + padding > activeBlock.GetSize())
		{
			if (!allowGrowing)
			{
				return nullptr;
			}
			// Grow same size as previous block, but make sure its enough space
			// NOTE: We use minimum size + alignment to make sure a
			// non aligned Grow allocates enough memory
			Grow(Math::Max(activeBlock.GetSize(), size + alignment), alignment);

			// Try again with new block
			return Allocate(size, alignment);
		}

		usedBlockSize += size + padding;
		return (u8*) (currentPtr) + padding;
	}

	void LinearAllocator::Reset()
	{
		usedBlockSize = 0;
		activeBlock.Free();
		for (HeapBlock& block : discardedBlocks)
		{
			block.Free();
		}
		discardedBlocks.Empty();
	}

	void LinearAllocator::Grow(sizet size, sizet /*align*/)
	{
		if (size > 0)    // Don't reserve an empty block
		{
			// Push last block for destructor deletion
			discardedBlocks.Add(Move(activeBlock));

			// TODO: Support aligned blocks
			activeBlock.Allocate(size);
			usedBlockSize = 0;
		}
	}
}    // namespace Rift::Memory
