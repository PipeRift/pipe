// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Math/Math.h"
#include "Memory/Blocks/HeapBlock.h"


namespace Rift::Memory
{
	/**
	 * LinearAllocator holds memory linearly in a block of memory.
	 * (Sometimes called ZoneAllocator if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	class CORE_API LinearAllocator
	{
	protected:
		HeapBlock activeBlock{};
		sizet usedBlockSize = 0;
		TArray<HeapBlock> discardedBlocks;
		bool allowGrowing = true;


	public:
		LinearAllocator(const sizet initialSize = 0, bool allowGrowing = true)
		    : activeBlock{initialSize}
		    , allowGrowing{allowGrowing}
		{}
		~LinearAllocator()
		{
			Reset();
		}

		void* Allocate(const sizet size);
		void* Allocate(const sizet size, const sizet alignment);

		void Free(void* ptr) {}

		void Reset();

		void Grow(sizet size, sizet align = 0);

		sizet GetUsedBlockSize() const
		{
			return usedBlockSize;
		}
		sizet GetBlockSize() const
		{
			return activeBlock.GetSize();
		}
		HeapBlock& GetBlock()
		{
			return activeBlock;
		}
		const HeapBlock& GetBlock() const
		{
			return activeBlock;
		}
		const TArray<HeapBlock>& GetDiscardedBlocks() const
		{
			return discardedBlocks;
		}

	private:
		// void GrowIfNeeded();
	};
}    // namespace Rift::Memory
