// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Memory/Arena.h"
#include "Pipe/Memory/BigBestFitArena.h"
#include "Pipe/Memory/Block.h"


namespace p
{
	class PIPE_API BigBestFitArena : public ChildArena
	{
	public:
		struct AllocationHeader
		{
			u8* end;
		};

		struct PIPE_API Slot
		{
			u8* start;
			u8* end;

			sizet Size() const
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
		Memory::Block block{};
		TArray<Slot> freeSlots{};
		bool pendingSort = false;
		sizet freeSize   = 0;


	public:
		BigBestFitArena(Arena* parent, const sizet initialSize = 1024);
		BigBestFitArena(const sizet initialSize = 1024) : BigBestFitArena(nullptr, initialSize) {}
		~BigBestFitArena() override;

		void* Alloc(const sizet size);
		void* Alloc(const sizet size, sizet alignment);
		bool Resize(void* ptr, sizet ptrSize, sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size);

		const Memory::Block& GetBlock() const
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
			return block.size - freeSize;
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
}    // namespace p