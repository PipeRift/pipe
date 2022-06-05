// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Core/Array.h"
#include "Core/Utility.h"
#include "Math/Math.h"
#include "Memory/Arenas/IArena.h"
#include "Memory/Blocks/HeapBlock.h"


namespace p::Memory
{
	/**
	 * LinearArena holds memory linearly in a block of memory.
	 * (Sometimes called ZoneArena if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	class LinearArena : public IArena
	{
	protected:
		HeapBlock activeBlock{};
		sizet usedBlockSize = 0;
		TArray<HeapBlock> discardedBlocks;
		bool allowGrowing = true;


	public:
		CORE_API LinearArena(const sizet initialSize = 0, bool allowGrowing = true)
		    : activeBlock{initialSize}, allowGrowing{allowGrowing}
		{}
		CORE_API ~LinearArena()
		{
			Reset();
		}
		LinearArena(const LinearArena&)     = delete;
		CORE_API LinearArena(LinearArena&&) = default;
		LinearArena& operator=(const LinearArena&) = delete;
		CORE_API LinearArena& operator=(LinearArena&&) = default;

		CORE_API void* Allocate(sizet size);
		CORE_API void* Allocate(sizet size, sizet alignment);

		CORE_API void Free(void* ptr, sizet size) {}

		CORE_API void Reset();

		CORE_API void Grow(sizet size, sizet align = 0);

		CORE_API sizet GetUsedBlockSize() const
		{
			return usedBlockSize;
		}
		CORE_API sizet GetBlockSize() const
		{
			return activeBlock.GetSize();
		}
		CORE_API HeapBlock& GetBlock()
		{
			return activeBlock;
		}
		CORE_API const HeapBlock& GetBlock() const
		{
			return activeBlock;
		}
		CORE_API const TArray<HeapBlock>& GetDiscardedBlocks() const
		{
			return discardedBlocks;
		}
	};
}    // namespace p::Memory
