// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Math/Math.h"
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
		u8* activeBlock = nullptr;
		TArray<u8*> fullBlocks;


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
				fullBlocks.Add(activeBlock);
				__Grow(size);
			}
		}

		void* Allocate(const size_t size);
		void* Allocate(const size_t size, const size_t alignment);

		void Free(void* ptr) = delete;

		void Reset();

		size_t GetUsedBlockSize() const
		{
			return usedBlockSize;
		}
		size_t GetMaxBlockSize() const
		{
			return maxBlockSize;
		}
		const void* GetActiveBlock() const
		{
			return activeBlock;
		}
		const TArray<u8*>& GetFullBlocks() const
		{
			return fullBlocks;
		}

	private:
		void __Grow(const size_t size)
		{
			usedBlockSize = 0;
			maxBlockSize = size;
			activeBlock = reinterpret_cast<u8*>(Rift::Alloc(maxBlockSize));
		}
	};

	template <bool allowGrowing>
	inline void* LinearAllocator<allowGrowing>::Allocate(const size_t size)
	{
		if (usedBlockSize + size > maxBlockSize)
		{
			if constexpr (!allowGrowing)
			{
				// TODO: Throw exception?
				return nullptr;
			}
			// Grow same size as previous block, but make sure its enough space
			Grow(Math::Max(maxBlockSize, size));
		}

		u8* const ptr = activeBlock + usedBlockSize;
		usedBlockSize += size;
		return ptr;
	}
	template <bool allowGrowing>
	inline void* LinearAllocator<allowGrowing>::Allocate(const size_t size, const size_t alignment)
	{
		if (usedBlockSize + size + alignment > maxBlockSize)
		{
			if constexpr (!allowGrowing)
			{
				// TODO: Throw exception?
				return nullptr;
			}
			// Grow same size as previous block, but make sure its enough space
			Grow(Math::Max(maxBlockSize, size));
		}

		void* ptr = activeBlock + usedBlockSize;
		size_t sizeLeft = maxBlockSize - usedBlockSize;
		std::align(alignment, size, ptr, sizeLeft);
		usedBlockSize = size_t(ptr) + size - size_t(activeBlock);
		return ptr;
	}

	template <bool allowGrowing>
	inline void LinearAllocator<allowGrowing>::Reset()
	{
		Rift::Free(activeBlock);
		for (void* block : fullBlocks)
		{
			Rift::Free(block);
		}
		fullBlocks.Empty();

		activeBlock = nullptr;
		usedBlockSize = 0;
		maxBlockSize = 0;
	}
}	 // namespace Rift::Memory
