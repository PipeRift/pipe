// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Math/Math.h"
#include "Memory/Arenas/IArena.h"
#include "Memory/Blocks/HeapBlock.h"
#include "Misc/Utility.h"


namespace Rift::Memory
{
	/**
	 * LinearArena holds memory linearly in a block of memory.
	 * (Sometimes called ZoneArena if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	class CORE_API LinearArena : public IArena
	{
	protected:
		HeapBlock activeBlock{};
		sizet usedBlockSize = 0;
		TArray<HeapBlock> discardedBlocks;
		bool allowGrowing = true;


	public:
		LinearArena(const sizet initialSize = 0, bool allowGrowing = true)
		    : activeBlock{initialSize}
		    , allowGrowing{allowGrowing}
		{}
		~LinearArena()
		{
			Reset();
		}
		LinearArena(const LinearArena&) = delete;
		LinearArena(LinearArena&&)      = default;
		LinearArena& operator=(const LinearArena&) = delete;
		LinearArena& operator=(LinearArena&&) = default;

		void* Allocate(const sizet size);
		void* Allocate(const sizet size, const sizet alignment);

		void Free(void* ptr, sizet size) {}

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
	};
}    // namespace Rift::Memory
