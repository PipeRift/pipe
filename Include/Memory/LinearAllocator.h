// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Memory/Alloc.h"


namespace Rift::Memory
{
	/**
	 * LinearAllocator holds memory linearly in a block of memory.
	 * Individual allocations can't be freed.
	 * It can be resized, but never smaller than its used size.
	 */
	template <bool allowGrowing = true>
	class LinearAllocator
	{
	protected:
		size_t usedBlockSize = 0;
		size_t maxBlockSize = 0;
		void* lastBlock = nullptr;
		TArray<void*> blocks;


	public:
		LinearAllocator(const size_t initialSize)
		{
			__Grow(initialSize > 0 ? initialSize : 1024);
		}
		~LinearAllocator()
		{
			Reset();
		}

		void Grow(const size_t size)
		{
			if (size > 0)	 // Dont reserve an empty block
			{
				// Push last block for destructor deletion
				blocks.Add(lastBlock);
				__Grow(size);
			}
		}

		void* Allocate(const size_t size, const size_t alignment = 0);

		void Free(void* ptr) = delete;

		void Reset();

	private:
		void __Grow(const size_t size)
		{
			usedBlockSize = 0;
			maxBlockSize = size;
			lastBlock = Rift::Alloc(maxBlockSize);
		}
	};


	template <bool allowGrowing>
	inline void* LinearAllocator<allowGrowing>::Allocate(const size_t size, const size_t alignment)
	{
		if (size + alignment < maxBlockSize - usedBlockSize)
		{
			if constexpr (!allowGrowing)
			{
				// TODO: Throw exception?
				return nullptr;
			}
			// Grow same size as previous block, but make sure its enough space
			Grow(Math::Max(maxBlockSize, size));
		}

		void* ptr = lastBlock + usedBlockSize;
		size_t sizeLeft = maxBlockSize - usedBlockSize;
		std::align(alignment, size, ptr, sizeLeft);

		usedBlockSize = ptr - lastBlock;
		return ptr;
	}

	template <bool allowGrowing>
	inline void LinearAllocator<allowGrowing>::Reset()
	{
		for (void* block : blocks)
		{
			Rift::Free(block);
		}
		Rift::Free(lastBlock);
		usedBlockSize = 0;
		maxBlockSize = 0;
	}
}	 // namespace Rift::Memory
