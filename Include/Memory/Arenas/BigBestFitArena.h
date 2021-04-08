// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Memory/Arenas/IArena.h"
#include "Memory/Blocks/HeapBlock.h"

#include <compare>


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
			u8* start;
			u8* end;

			auto operator<=>(const Slot& other) const
			{
				return sizet(end - start) <=> sizet(other.end - other.start);
			}
			auto operator<=>(sizet other) const
			{
				return sizet(end - start) <=> other;
			}
			auto operator==(const Slot& other) const
			{
				return sizet(end - start) == sizet(other.end - other.start);
			}
			auto operator==(sizet other) const
			{
				return sizet(end - start) == other;
			}
		};

	protected:
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


	/*inline auto operator<=>(const BigBestFitArena::Slot& one, const BigBestFitArena::Slot& other)
	{
	    return sizet(one.end - one.start) <=> sizet(other.end - other.start);
	}
	inline auto operator<=>(const BigBestFitArena::Slot& one, sizet other)
	{
	    return sizet(one.end - one.start) <=> other;
	}
	inline auto operator<=>(sizet one, const BigBestFitArena::Slot& other)
	{
	    return one <=> sizet(other.end - other.start);
	}

	// Operator <=> doesnt implement == in this case
	inline auto operator==(const BigBestFitArena::Slot& one, const BigBestFitArena::Slot& other)
	{
	    return sizet(one.end - one.start) == sizet(other.end - other.start);
	}
	inline auto operator==(const BigBestFitArena::Slot& one, sizet other)
	{
	    return sizet(one.end - one.start) == other;
	}*/
}    // namespace Rift::Memory
