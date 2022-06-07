// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"
#include "Pipe/Memory/Arenas/IArena.h"
#include "Pipe/Memory/Blocks/HeapBlock.h"


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
		PIPE_API LinearArena(const sizet initialSize = 0, bool allowGrowing = true)
		    : activeBlock{initialSize}, allowGrowing{allowGrowing}
		{}
		PIPE_API ~LinearArena()
		{
			Reset();
		}
		LinearArena(const LinearArena&)     = delete;
		PIPE_API LinearArena(LinearArena&&) = default;
		LinearArena& operator=(const LinearArena&) = delete;
		PIPE_API LinearArena& operator=(LinearArena&&) = default;

		PIPE_API void* Allocate(sizet size);
		PIPE_API void* Allocate(sizet size, sizet alignment);

		PIPE_API void Free(void* ptr, sizet size) {}

		PIPE_API void Reset();

		PIPE_API void Grow(sizet size, sizet align = 0);

		PIPE_API sizet GetUsedBlockSize() const
		{
			return usedBlockSize;
		}
		PIPE_API sizet GetBlockSize() const
		{
			return activeBlock.GetSize();
		}
		PIPE_API HeapBlock& GetBlock()
		{
			return activeBlock;
		}
		PIPE_API const HeapBlock& GetBlock() const
		{
			return activeBlock;
		}
		PIPE_API const TArray<HeapBlock>& GetDiscardedBlocks() const
		{
			return discardedBlocks;
		}
	};
}    // namespace p::Memory
