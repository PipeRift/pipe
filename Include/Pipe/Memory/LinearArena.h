// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/Block.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
	/**
	 * LinearArena holds memory linearly in a block of memory.
	 * (Sometimes called ZoneArena if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	class LinearArena : public ChildArena
	{
	protected:
		Memory::Block activeBlock{};
		sizet usedBlockSize = 0;
		TArray<Memory::Block> discardedBlocks;
		bool allowGrowing = true;


	public:
		PIPE_API LinearArena(
		    Arena* parent, const sizet firstBlockSize = Memory::MB, bool allowGrowing = true)
		    : ChildArena(parent)
		    , activeBlock{p::Alloc(GetParentArena(), firstBlockSize), firstBlockSize}
		    , allowGrowing{allowGrowing}
		{
			Interface<LinearArena, &LinearArena::Alloc, &LinearArena::Alloc, &LinearArena::Resize,
			    &LinearArena::Free>();
		}
		PIPE_API LinearArena(const sizet firstBlockSize = Memory::MB, bool allowGrowing = true)
		    : LinearArena(nullptr, firstBlockSize, allowGrowing)
		{}
		PIPE_API ~LinearArena() override
		{
			Reset();
		}

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
			return activeBlock.size;
		}
		PIPE_API Memory::Block& GetBlock()
		{
			return activeBlock;
		}
		PIPE_API const Memory::Block& GetBlock() const
		{
			return activeBlock;
		}
		PIPE_API const TArray<Memory::Block>& GetDiscardedBlocks() const
		{
			return discardedBlocks;
		}
	};
}    // namespace p
