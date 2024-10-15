// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Checks.h"
#include "Pipe/Memory/Alloc.h"
#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/Memory.h"


namespace p
{
	struct PIPE_API LinearBlock
	{
		sizet count       = 0;    // Counts number of live allocations
		LinearBlock* next = nullptr;
		LinearBlock* last = nullptr;
		void* unaligned   = nullptr;    // Pointer where this block was allocated before alignment

		constexpr u8* End(sizet blockSize) const
		{
			return (u8*)unaligned + blockSize + sizeof(LinearBlock);
		}
	};

	template<sizet blockSize>
	struct PIPE_API LinearBasePool
	{
		void* insert           = nullptr;    // Pointer where to insert new allocations
		LinearBlock* freeBlock = nullptr;


	private:
		void AllocateBlock(Arena& parentArena);
		void FreeBlock(Arena& parentArena, LinearBlock* block);

	public:
		void* Alloc(Arena& parentArena, sizet size, sizet align);
		void Free(Arena& parentArena, void* ptr, sizet size);

		void Release(Arena& parentArena);

		static constexpr sizet GetBlockSize()
		{
			return blockSize;
		}

		static constexpr sizet GetAllocatedBlockSize()
		{
			return blockSize + sizeof(LinearBlock);
		}

		void* GetBlockEnd(LinearBlock* block) const
		{
			return (u8*)block->unaligned + GetAllocatedBlockSize();
		}
	};

	struct PIPE_API LinearSmallPool : public LinearBasePool<1 * Memory::MB>
	{
		static constexpr sizet minSize = 0;
		static constexpr sizet maxSize = 8 * Memory::KB;
	};

	struct PIPE_API LinearMediumPool : public LinearBasePool<4 * Memory::MB>
	{
		static constexpr sizet minSize = LinearSmallPool::maxSize;
		static constexpr sizet maxSize = 512 * Memory::KB;
	};

	struct PIPE_API LinearBigPool : public LinearBasePool<16 * Memory::MB>
	{
		static constexpr sizet minSize = LinearMediumPool::maxSize;
		static constexpr sizet maxSize = 4 * Memory::MB;
		// Block size is the size of the allocation
	};


	/**
	 * LinearArena holds memory linearly in a block of memory.
	 * (Sometimes called ZoneArena if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	struct PIPE_API MultiLinearArena : public ChildArena
	{
	protected:
		LinearSmallPool smallPool;
		LinearMediumPool mediumPool;
		LinearBigPool bigPool;


	public:
		MultiLinearArena(Arena& parentArena = GetCurrentArena()) : ChildArena(&parentArena)
		{
			Interface<MultiLinearArena>();
		}
		~MultiLinearArena() override
		{
			Release();
		}

		void* Alloc(sizet size)
		{
			// TODO: Resolve a reasonable align size based on allocation size
			return Alloc(size, alignof(std::max_align_t));
		}
		void* Alloc(sizet size, sizet align);
		bool Realloc(void* ptr, sizet ptrSize, sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size);


		void Release();

		void Grow(sizet size, sizet align = 0);
	};
}    // namespace p
