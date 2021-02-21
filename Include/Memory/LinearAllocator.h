// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Math/Math.h"
#include "Memory/Blocks/MallocBlock.h"


namespace Rift::Memory
{
	/**
	 * LinearAllocator holds memory linearly in a block of memory.
	 * (Sometimes called ZoneAllocator if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	template <bool allowGrowing = true>
	class LinearAllocator
	{
	protected:
		MallocBlock activeBlock{};
		size_t usedBlockSize = 0;
		TArray<MallocBlock> discardedBlocks;


	public:
		LinearAllocator(const size_t initialSize = 0) : activeBlock{initialSize} {}
		~LinearAllocator()
		{
			Reset();
		}

		void Grow(const size_t size)
		{
			if (size > 0)    // Don't reserve an empty block
			{
				// Push last block for destructor deletion
				discardedBlocks.Add(Move(activeBlock));

				usedBlockSize = 0;
				activeBlock.Allocate(size);
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
		size_t GetBlockSize() const
		{
			return activeBlock.GetSize();
		}
		MallocBlock& GetBlock()
		{
			return activeBlock;
		}
		const MallocBlock& GetBlock() const
		{
			return activeBlock;
		}
		const TArray<MallocBlock>& GetDiscardedBlocks() const
		{
			return discardedBlocks;
		}
	};

	template <bool allowGrowing>
	inline void* LinearAllocator<allowGrowing>::Allocate(const size_t size)
	{
		if (usedBlockSize + size > activeBlock.GetSize())
		{
			if constexpr (!allowGrowing)
			{
				// TODO: Throw exception?
				return nullptr;
			}
			// Grow same size as previous block, but make sure its enough space
			Grow(Math::Max(activeBlock.GetSize(), size));
		}

		u8* const ptr = reinterpret_cast<u8*>(activeBlock.GetData()) + usedBlockSize;
		usedBlockSize += size;
		return ptr;
	}
	template <bool allowGrowing>
	inline void* LinearAllocator<allowGrowing>::Allocate(const size_t size, const size_t alignment)
	{
		if (usedBlockSize + size + alignment > activeBlock.GetSize())
		{
			if constexpr (!allowGrowing)
			{
				// TODO: Throw exception?
				return nullptr;
			}
			// Grow same size as previous block, but make sure its enough space
			Grow(Math::Max(activeBlock.GetSize(), size));
		}

		void* ptr = activeBlock.GetData() + usedBlockSize;

		size_t sizeLeft = activeBlock.GetSize() - usedBlockSize;
		std::align(alignment, size, ptr, sizeLeft);
		usedBlockSize = activeBlock.GetSize() - sizeLeft;
		return ptr;
	}

	template <bool allowGrowing>
	inline void LinearAllocator<allowGrowing>::Reset()
	{
		usedBlockSize = 0;
		activeBlock.Free();
		for (MallocBlock& block : discardedBlocks)
		{
			block.Free();
		}
		discardedBlocks.Empty();
	}
}    // namespace Rift::Memory
