// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/LinearArena.h"

#include "Pipe/Memory/Block.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
		template<sizet blockSize>
	void LinearBasePool::AllocateBlock(Arena& parentArena)
	{
		LinearBlock* lastBlock = currentBlock;

		// Allocate aligning by blockSize
		void* ptr = p::Alloc(parentArena, blockSize + sizeof(LinearBlock), blockSize);

		void* blockPtr = (u8*)ptr + GetAlignmentPadding(ptr, blockSize);
		currentBlock = new (blockPtr) LinearBlock();
		currentBlock->insert = currentBlock + 1;
		currentBlock->last = lastBlock;
		currentBlock->unaligned = ptr;

		lastBlock->next = currentBlock;
	}

	template<sizet blockSize>
	void LinearBasePool::FreeBlock(Arena& parentArena, LinearBlock* block)
	{
		Check(block);
		if (currentBlock == block)
		{
			currentBlock = block->last;
		}
		else if (block->next) // currentBlock never has next block, only last
		{
			block->next->last = block->last;
		}

		if (block->last)
		{
			block->last->next = block->next;
		}

		p::Free(parentArena, block->unaligned, blockSize + sizeof(LinearBlock));
	}

	void* LinearSmallPool::Alloc(Arena& parentArena, sizet size, sizet align)
	{
		u8*& insert = (u8*&)currentBlock->insert;

		// Not enough space in current block?
		sizet padding = GetAlignmentPadding(insert, align);
		if (insert + size + padding > currentBlock->End<blockSize>())
		{
			AllocateBlock<blockSize>(parentArena);

			// Find alignment on the new block. Guaranteed to fit
			padding = GetAlignmentPadding(insert, align);
		}

		u8* const ptr = insert + padding;
		insert = ptr + size;
		return ptr;
	}

	void LinearSmallPool::Free(Arena& parentArena, void* ptr, sizet size) {
		// TODO
	}

	void* LinearArena::Alloc(sizet size, sizet align)
	{
		if (size < smallPool.maxSize)
		{
			return smallPool.Alloc(GetParentArena(), size, align);
		}
		else if (size < mediumPool.maxSize)
		{
			//return mediumPool.Alloc(GetParentArena(), size, align);
		}
		else if (size < bigPool.maxSize)
		{
			//return bigPool.Alloc(GetParentArena(), size, align);
		}
		else
		{
			return p::Alloc(GetParentArena(), size, align);
		}
	}

	void LinearArena::Free(void* ptr, sizet size)
	{
		if (size < smallPool.maxSize)
		{
			smallPool.Free(GetParentArena(), ptr, size);
		}
		else if (size < mediumPool.maxSize)
		{
			//mediumPool.Free(GetParentArena(), ptr, size);
		}
		else if (size < bigPool.maxSize)
		{
			//bigPool.Free(GetParentArena(), ptr, size);
		}
		else
		{
			p::Free(GetParentArena(), ptr, size);
		}
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
