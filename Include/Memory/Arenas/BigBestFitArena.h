// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "PCH.h"

#include "Core/Array.h"
#include "Memory/Arenas/IArena.h"
#include "Memory/Blocks/HeapBlock.h"


namespace pipe::Memory
{
	class CORE_API BigBestFitArena : public IArena
	{
	public:
		struct AllocationHeader
		{
			u8* end;
		};

		struct CORE_API Slot
		{
			u8* start;
			u8* end;

			sizet GetSize() const
			{
				return sizet(end - start);
			}

			auto operator==(const Slot& other) const
			{
				return sizet(end - start) == sizet(other.end - other.start);
			}
			auto operator<(const Slot& other) const
			{
				return sizet(end - start) < sizet(other.end - other.start);
			}
			auto operator>(const Slot& other) const
			{
				return sizet(end - start) > sizet(other.end - other.start);
			}
			auto operator<=(const Slot& other) const
			{
				return sizet(end - start) <= sizet(other.end - other.start);
			}
			auto operator>=(const Slot& other) const
			{
				return sizet(end - start) >= sizet(other.end - other.start);
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
		~BigBestFitArena() override {}

		void* Allocate(const sizet size) final;
		void* Allocate(const sizet size, sizet alignment) final;

		void Free(void* ptr, sizet size) final;

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
}    // namespace pipe::Memory
