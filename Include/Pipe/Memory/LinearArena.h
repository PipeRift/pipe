// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Limits.h"
#include "Pipe/Core/Utility.h"
#include "Pipe/Math/Math.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/Block.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
	struct PIPE_API LinearBlock
	{
		void* insert = nullptr; // Pointer where to insert new allocations
		sizet counter = 0; // Counts number of live allocations
		LinearBlock* next = nullptr;
		LinearBlock* last = nullptr;
		void* unaligned = nullptr; // Pointer where this block was allocated before alignment

		template<sizet blockSize>
		u8* End() const { return (u8*)unaligned + blockSize + sizeof(LinearBlock); }
	};

	struct PIPE_API LinearBasePool
	{
		LinearBlock* currentBlock = nullptr;


		template<sizet blockSize>
		void AllocateBlock(Arena& parentArena);
		template<sizet blockSize>
		void FreeBlock(Arena& parentArena, LinearBlock* block);
	};

	struct PIPE_API LinearSmallPool : public LinearBasePool
	{
		static constexpr sizet maxSize   = 8 * Memory::KB;
		static constexpr sizet minSize   = 0;
		static constexpr sizet blockSize = 1 * Memory::MB;

		void* Alloc(Arena& parentArena, sizet size, sizet align);
		void Free(Arena& parentArena, void* ptr, sizet size);
	};

	struct PIPE_API LinearMediumPool
	{
		static constexpr sizet maxSize   = 512 * Memory::KB;
		static constexpr sizet minSize   = LinearSmallPool::maxSize;
		static constexpr sizet blockSize = 4 * Memory::MB;
	};

	struct PIPE_API LinearBigPool
	{
		static constexpr sizet maxSize = 4 * Memory::MB;
		static constexpr sizet minSize = LinearMediumPool::maxSize;
		static constexpr sizet blockSize = 16 * Memory::MB;
		// Block size is the size of the allocation
	};


	/**
	 * LinearArena holds memory linearly in a block of memory.
	 * (Sometimes called ZoneArena if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	class PIPE_API LinearArena : public ChildArena
	{
	protected:
		LinearSmallPool smallPool;
		LinearMediumPool mediumPool;
		LinearBigPool bigPool;

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

		void* Alloc(sizet size)
		{
			// TODO: Resolve a reasonable align size based on allocation size
			return Alloc(size, alignof(std::max_align_t));
		}
		void* Alloc(sizet size, sizet align);
		bool Resize(void* ptr, sizet ptrSize, sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size);


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
