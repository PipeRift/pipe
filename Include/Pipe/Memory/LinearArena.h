// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Blocks/HeapBlock.h"
#include "Pipe/Memory/IArena.h"


namespace p
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
		Memory::HeapBlock activeBlock{};
		sizet usedBlockSize = 0;
		TArray<Memory::HeapBlock> discardedBlocks;
		bool allowGrowing = true;


	public:
		PIPE_API LinearArena(const sizet initialSize = 0, bool allowGrowing = true)
		    : activeBlock{initialSize}, allowGrowing{allowGrowing}
		{
			SetupInterface(
			    &LinearArena::Alloc, &LinearArena::Alloc, &LinearArena::Resize, &LinearArena::Free);
		}
		PIPE_API ~LinearArena() override
		{
			Reset();
		}
		LinearArena(const LinearArena&)     = delete;
		PIPE_API LinearArena(LinearArena&&) = default;
		LinearArena& operator=(const LinearArena&) = delete;
		PIPE_API LinearArena& operator=(LinearArena&&) = default;

		PIPE_API void* Alloc(sizet size);
		PIPE_API void* Alloc(sizet size, sizet align);
		PIPE_API bool Resize(void* ptr, sizet ptrSize, sizet size)
		{
			return false;
		}
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
		PIPE_API Memory::HeapBlock& GetBlock()
		{
			return activeBlock;
		}
		PIPE_API const Memory::HeapBlock& GetBlock() const
		{
			return activeBlock;
		}
		PIPE_API const TArray<Memory::HeapBlock>& GetDiscardedBlocks() const
		{
			return discardedBlocks;
		}
	};
}    // namespace p
