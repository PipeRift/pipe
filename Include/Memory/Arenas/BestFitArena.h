// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Memory/Arenas/IArena.h"
#include "Memory/Blocks/HeapBlock.h"


namespace Rift::Memory
{
	class CORE_API BestFitArena : public IArena
	{
	public:
		struct Slot
		{
			u8* start;
			sizet size;

			u8* GetEnd() const;
		};

	protected:
		// TODO: Support growing multiple blocks
		HeapBlock block{};
		TArray<Slot> freeSlots{};
		bool pendingSort = false;
		sizet freeSize   = 0;


	public:
		BestFitArena(const sizet initialSize = 1024);
		~BestFitArena() {}

		void* Allocate(const sizet size);
		void* Allocate(const sizet size, sizet alignment);

		void Free(void* ptr, sizet size);

		const HeapBlock& GetBlock() const
		{
			return block;
		}

		bool Contains(void* ptr)
		{
			return block.Contains(ptr);
		}
		sizet GetFreeSize()
		{
			return freeSize;
		}

		sizet GetUsedSize()
		{
			return block.GetSize() - freeSize;
		}

		const TArray<Slot>& GetFreeSlots() const
		{
			return freeSlots;
		}

	private:
		i32 FindSmallestSlot(sizet neededSize);
		void ReduceSlot(
		    i32 slotIndex, Slot& slot, u8* const allocationStart, u8* const allocationEnd);
		void AbsorbFreeSpace(u8* const allocationStart, u8* const allocationEnd);
	};


	inline u8* BestFitArena::Slot::GetEnd() const
	{
		return start + size;
	}
}    // namespace Rift::Memory
