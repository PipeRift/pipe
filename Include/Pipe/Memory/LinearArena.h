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
	class PIPE_API LinearArena : public ChildArena
	{
	protected:
		Memory::Block activeBlock{};
		sizet usedBlockSize = 0;
		TArray<Memory::Block> discardedBlocks;
		bool allowGrowing = true;


	public:
		LinearArena(
		    Arena* parent, const sizet firstBlockSize = Memory::MB, bool allowGrowing = true)
		    : ChildArena(parent)
		    , activeBlock{p::Alloc(GetParentArena(), firstBlockSize), firstBlockSize}
		    , allowGrowing{allowGrowing}
		{
			Interface<LinearArena, &LinearArena::Alloc, &LinearArena::Alloc, &LinearArena::Resize,
			    &LinearArena::Free>();
		}
		LinearArena(const sizet firstBlockSize = Memory::MB, bool allowGrowing = true)
		    : LinearArena(nullptr, firstBlockSize, allowGrowing)
		{}
		~LinearArena() override
		{
			Reset();
		}

		void* Alloc(sizet size);
		void* Alloc(sizet size, sizet align);
		bool Resize(void* ptr, sizet ptrSize, sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size) {}


		void Reset();

		void Grow(sizet size, sizet align = 0);

		sizet GetUsedBlockSize() const
		{
			return usedBlockSize;
		}
		sizet GetBlockSize() const
		{
			return activeBlock.size;
		}
		Memory::Block& GetBlock()
		{
			return activeBlock;
		}
		const Memory::Block& GetBlock() const
		{
			return activeBlock;
		}
		const TArray<Memory::Block>& GetDiscardedBlocks() const
		{
			return discardedBlocks;
		}
	};
}    // namespace p
