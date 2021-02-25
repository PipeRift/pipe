// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "PCH.h"

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
			u8* start;
			u8* end;
		};

		struct SlotArray
		{
			Slot* data = nullptr;
			u32 size   = 0;


			void SetData(void* address)
			{
				data = static_cast<Slot*>(address);
			}

			void Add(Slot&& slot);
			void RemoveSwap(u32 index);
			void SortBySize();
			u32 FindSmallest(sizet minSlotSize);

			Slot& GetRef(u32 index)
			{
				return data[index];
			}

			bool IsValidIndex(u32 index)
			{
				return index >= 0 && index < size;
			}
		};

		// TODO: Make sure minAlignment is multiple of 2. Unlikely to change though
		static constexpr sizet minAlignment = sizeof(AllocationHeader);
		HeapBlock block{};
		SlotArray freeSlots{};
		bool pendingSort = false;


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

		u32 GetNumFreeSlots() const
		{
			return freeSlots.size;
		}

	private:
		AllocationHeader* GetHeader(void* ptr)
		{
			return reinterpret_cast<AllocationHeader*>(
			    static_cast<u8*>(ptr) - sizeof(AllocationHeader));
		}
	};
}    // namespace Rift::Memory
