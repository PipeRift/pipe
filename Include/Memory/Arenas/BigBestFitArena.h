// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Memory/Arenas/IArena.h"
#include "Memory/Blocks/HeapBlock.h"


namespace Rift::Memory
{
	class CORE_API BigBestFitArena : public IArena
	{
	public:
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

	protected:
		// TODO: Make sure minAlignment is multiple of 2. Unlikely to change though
		static constexpr sizet minAlignment = sizeof(AllocationHeader);
		// TODO: Support growing multiple blocks
		HeapBlock block{};
		TArray<Slot> freeSlots{};
		bool pendingSort = false;
		sizet freeSize   = 0;


	public:
		BigBestFitArena(const sizet initialSize = 1024);
		~BigBestFitArena() {}

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

		void* GetAllocationStart(void* ptr) const
		{
			return GetHeader(ptr);
		}
		void* GetAllocationEnd(void* ptr) const
		{
			return GetHeader(ptr)->end;
		}

	private:
		AllocationHeader* GetHeader(void* ptr) const
		{
			return reinterpret_cast<AllocationHeader*>(
			    static_cast<u8*>(ptr) - sizeof(AllocationHeader));
		}

		i32 FindSmallestSlot(sizet size);
		void ReduceSlot(
		    i32 slotIndex, Slot& slot, u8* const allocationStart, u8* const allocationEnd);
		void AbsorbFreeSpace(u8* const allocationStart, u8* const allocationEnd);
	};
}    // namespace Rift::Memory
