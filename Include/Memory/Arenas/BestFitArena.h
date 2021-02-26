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
	protected:
		struct AllocationHeader
		{
			u8* end;
		};

		struct Slot
		{
			// TODO: Test use of size instead of end for more efficient sort and search
			u8* start;
			u8* end;
		};


		// TODO: Make sure minAlignment is multiple of 2. Unlikely to change though
		static constexpr sizet minAlignment = sizeof(AllocationHeader);
		// TODO: Support growing multiple blocks
		HeapBlock block{};
		TArray<Slot> freeSlots{};
		bool pendingSort     = false;
		sizet availableSpace = 0;


	public:
		BestFitArena(const sizet initialSize = 1024);
		~BestFitArena() {}

		void* Allocate(const sizet size);
		void* Allocate(const sizet size, sizet alignment);

		void Free(void* ptr);

		const HeapBlock& GetBlock() const
		{
			return block;
		}

		bool Contains(void* ptr)
		{
			return block.Contains(ptr);
		}
		sizet GetAvailableSpace()
		{
			return availableSpace;
		}

		sizet GetUsedSpace()
		{
			return block.GetSize() - availableSpace;
		}

		const TArray<Slot>& GetFreeSlots() const
		{
			return freeSlots;
		}

	private:
		AllocationHeader* GetHeader(void* ptr)
		{
			return reinterpret_cast<AllocationHeader*>(
			    static_cast<u8*>(ptr) - sizeof(AllocationHeader));
		}

		i32 FindSmallestSlot(sizet size);
		void ReduceSlot(i32 slotIndex, Slot& slot, u8* const allocationEnd);
		void AbsorbFreeSpace(u8* const allocationStart, u8* const allocationEnd);
	};
}    // namespace Rift::Memory
