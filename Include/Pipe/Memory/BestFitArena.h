// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Memory/Blocks/HeapBlock.h"
#include "Pipe/Memory/IArena.h"


namespace p
{
	class PIPE_API BestFitArena : public IArena
	{
	public:
		struct Slot
		{
			u8* start;
			sizet size;

			u8* GetEnd() const;

			auto operator==(const Slot& other) const
			{
				return size == other.size;
			}
			auto operator<(const Slot& other) const
			{
				return size < other.size;
			}
			auto operator>(const Slot& other) const
			{
				return size > other.size;
			}
			auto operator<=(const Slot& other) const
			{
				return size <= other.size;
			}
			auto operator>=(const Slot& other) const
			{
				return size >= other.size;
			}
		};

	protected:
		// TODO: Support growing multiple blocks
		Memory::HeapBlock block{};
		TArray<Slot> freeSlots{};
		bool pendingSort = false;
		sizet freeSize   = 0;


	public:
		BestFitArena(const sizet initialSize = 1024);
		~BestFitArena() {}

		void* Alloc(const sizet size);
		void* Alloc(const sizet size, sizet align);
		bool Resize(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size);

		const Memory::HeapBlock& GetBlock() const
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
}    // namespace p
