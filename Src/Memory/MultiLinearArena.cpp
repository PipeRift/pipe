// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Memory/MultiLinearArena.h"

#include "Pipe/Memory/Block.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
	template<sizet blockSize>
	void* LinearBasePool<blockSize>::Alloc(Arena& parentArena, sizet size, sizet align)
	{
		u8* allocEnd = (u8*)insert + size + GetAlignmentPadding(insert, align);

		// Not enough space in current block?
		if (!freeBlock || allocEnd > GetBlockEnd(freeBlock)) [[unlikely]]
		{
			AllocateBlock(parentArena);
			// Recalculate allocation with new block
			allocEnd = (u8*)insert + size + GetAlignmentPadding(insert, align);
		}

		insert = allocEnd;
		++freeBlock->count;
		return (u8*)insert - size;
	}

	template<sizet blockSize>
	void LinearBasePool<blockSize>::Free(Arena& parentArena, void* ptr, sizet size)
	{
		auto* block = static_cast<LinearBlock*>(GetAlignedBlock(ptr, GetBlockSize()));
		if (block) [[likely]]
		{
			--block->count;
			if (block->count <= 0) [[unlikely]]
			{
				FreeBlock(parentArena, block);
			}
		}
	}

	template<sizet blockSize>
	void LinearBasePool<blockSize>::AllocateBlock(Arena& parentArena)
	{
		LinearBlock* const lastBlock = freeBlock;

		// Allocate aligning by blockSize
		void* ptr = p::Alloc(parentArena, blockSize + sizeof(LinearBlock), GetBlockSize());

		void* blockPtr = (u8*)ptr + GetAlignmentPadding(ptr, GetBlockSize());
		freeBlock      = new (blockPtr) LinearBlock();
		insert         = freeBlock + 1;

		freeBlock->last      = lastBlock;
		freeBlock->unaligned = ptr;

		if (lastBlock)
		{
			lastBlock->next = freeBlock;
		}
	}

	template<sizet blockSize>
	void LinearBasePool<blockSize>::FreeBlock(Arena& parentArena, LinearBlock* block)
	{
		Check(block);
		if (freeBlock == block)
		{
			freeBlock = block->last;
			insert    = freeBlock + 1;
		}
		else if (block->next)    // currentBlock never has next block, only last
		{
			block->next->last = block->last;
		}

		if (block->last)
		{
			block->last->next = block->next;
		}

		p::Free(parentArena, block->unaligned, GetAllocatedBlockSize());
	}

	template<sizet blockSize>
	void LinearBasePool<blockSize>::Release(Arena& parentArena)
	{
		insert             = nullptr;
		LinearBlock* block = freeBlock;
		while (block)
		{
			LinearBlock* const blockToRemove = block;
			block                            = block->last;
			p::Free(parentArena, blockToRemove->unaligned, GetAllocatedBlockSize());
		}
		freeBlock = nullptr;
	}

	void* MultiLinearArena::Alloc(sizet size, sizet align)
	{
		if (size < smallPool.maxSize)
		{
			return smallPool.Alloc(GetParentArena(), size, align);
		}
		else if (size < mediumPool.maxSize)
		{
			// return mediumPool.Alloc(GetParentArena(), size, align);
		}
		else if (size < bigPool.maxSize)
		{
			// return bigPool.Alloc(GetParentArena(), size, align);
		}
		else
		{
			return p::Alloc(GetParentArena(), size, align);
		}
	}

	void MultiLinearArena::Free(void* ptr, sizet size)
	{
		if (size < smallPool.maxSize)
		{
			smallPool.Free(GetParentArena(), ptr, size);
		}
		else if (size < mediumPool.maxSize)
		{
			// mediumPool.Free(GetParentArena(), ptr, size);
		}
		else if (size < bigPool.maxSize)
		{
			// bigPool.Free(GetParentArena(), ptr, size);
		}
		else
		{
			p::Free(GetParentArena(), ptr, size);
		}
	}

	void MultiLinearArena::Release()
	{
		smallPool.Release(GetParentArena());
		mediumPool.Release(GetParentArena());
		bigPool.Release(GetParentArena());
	}
}    // namespace p
