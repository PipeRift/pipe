// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Memory/MemoryStats.h"
#include "PipeMemory.h"


namespace p
{
#pragma region Dummy Arena
	/** This is an Arena example.
	 * It serves the single purpose of documenting an Arena's interface.
	 * Not intended to be used.
	 */
	class P_API DummyArena : public Arena
	{
	public:
		DummyArena()
		{
			Interface<DummyArena>();
		}
		~DummyArena() {}

		void* Alloc(const sizet size)
		{
			return nullptr;
		}
		void* Alloc(const sizet size, const sizet alignment)
		{
			return nullptr;
		}
		bool Realloc(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size) {}

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<DummyArena>();
		}
	};
#pragma endregion Dummy Arena

#pragma region Heap Arena
	class P_API HeapArena : public Arena
	{
	private:
		MemoryStats stats;

	public:
		HeapArena();

		void* Alloc(const sizet size);
		void* Alloc(const sizet size, const sizet align);
		bool Realloc(void* ptr, const sizet ptrSize, const sizet size);
		void Free(void* ptr, sizet size);

		const MemoryStats* GetStats() const override
		{
			return &stats;
		}

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<HeapArena>();
		}
	};
#pragma endregion Heap Arena

#pragma region Mono Linear
	/**
	 * LinearArena holds memory linearly in a block of memory.
	 * (Sometimes called ZoneArena if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	class P_API MonoLinearArena : public ChildArena
	{
	private:
		MemoryStats stats;

	protected:
		void* insert = nullptr;
		sizet count  = 0;
		ArenaBlock block{};
		bool selfAllocated = false;


	public:
		MonoLinearArena(ArenaBlock externalBlock, Arena& parentArena = GetCurrentArena());
		MonoLinearArena(const sizet blockSize = Memory::MB, Arena& parentArena = GetCurrentArena());
		~MonoLinearArena()
		{
			Release(false);
		}

		void* Alloc(sizet size);

		void* Alloc(sizet size, sizet align);

		bool Realloc(void* ptr, sizet ptrSize, sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size);

		void Release(bool keepIfSelfAllocated = true);

		sizet GetAvailableMemory() const override
		{
			return block.size;
		}
		void GetBlocks(TArray<ArenaBlock>& outBlocks) const override
		{
			outBlocks.Add(block);
		}

		const MemoryStats* GetStats() const override
		{
			return &stats;
		}

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<MonoLinearArena>();
		}
	};

	// TMonoLinearArena works like a MonoLinearArena but providing a block on the stack as the block
	// to use
	template<sizet blockSize = Memory::MB>
	class P_API TMonoLinearArena : public MonoLinearArena
	{
		u8 buffer[blockSize];


		TMonoLinearArena(Arena& parentArena = GetCurrentArena())
		    : MonoLinearArena(ArenaBlock{buffer, blockSize}, parentArena)
		{}
	};
#pragma endregion Mono Linear

#pragma region Multi Linear
	namespace Details
	{
		struct P_API LinearBlock
		{
			sizet count       = 0;    // Counts number of live allocations
			LinearBlock* next = nullptr;
			LinearBlock* last = nullptr;
			void* unaligned = nullptr;    // Pointer where this block was allocated before alignment

			constexpr u8* End(sizet blockSize) const
			{
				return (u8*)unaligned + blockSize + sizeof(LinearBlock);
			}
		};

		template<sizet blockSize>
		struct P_API LinearBasePool
		{
			void* insert           = nullptr;    // Pointer where to insert new allocations
			LinearBlock* freeBlock = nullptr;


		private:
			void AllocateBlock(Arena& parentArena);
			void FreeBlock(Arena& parentArena, LinearBlock* block);

		public:
			void* Alloc(Arena& parentArena, sizet size, sizet align);
			void Free(Arena& parentArena, void* ptr, sizet size);

			void Release(Arena& parentArena);

			static constexpr sizet GetBlockSize()
			{
				return blockSize;
			}

			static constexpr sizet GetAllocatedBlockSize()
			{
				return blockSize + sizeof(LinearBlock);
			}

			void* GetBlockEnd(LinearBlock* block) const
			{
				return (u8*)block->unaligned + GetAllocatedBlockSize();
			}
		};

		struct P_API LinearSmallPool : public LinearBasePool<1 * Memory::MB>
		{
			static constexpr sizet minSize = 0;
			static constexpr sizet maxSize = 8 * Memory::KB;
		};

		struct P_API LinearMediumPool : public LinearBasePool<4 * Memory::MB>
		{
			static constexpr sizet minSize = LinearSmallPool::maxSize;
			static constexpr sizet maxSize = 512 * Memory::KB;
		};

		struct P_API LinearBigPool : public LinearBasePool<16 * Memory::MB>
		{
			static constexpr sizet minSize = LinearMediumPool::maxSize;
			static constexpr sizet maxSize = 4 * Memory::MB;
			// Block size is the size of the allocation
		};
	}    // namespace Details

	/**
	 * LinearArena holds memory linearly in a block of memory.
	 * (Sometimes called ZoneArena if it resizes)
	 * Individual allocations can't be freed. It can
	 * be resized, but never smaller than its used size.
	 */
	struct P_API MultiLinearArena : public ChildArena
	{
	protected:
		Details::LinearSmallPool smallPool;
		Details::LinearMediumPool mediumPool;
		Details::LinearBigPool bigPool;


	public:
		MultiLinearArena(Arena& parentArena = GetCurrentArena());
		~MultiLinearArena()
		{
			Release();
		}

		void* Alloc(sizet size);

		void* Alloc(sizet size, sizet align);

		bool Realloc(void* ptr, sizet ptrSize, sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size);

		void Release();

		void Grow(sizet size, sizet align = 0);

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<MultiLinearArena>();
		}
	};
#pragma endregion Multi Linear

#pragma region Best Fit Arena
	class P_API BestFitArena : public ChildArena
	{
	public:
		struct Slot
		{
			u8* start;
			sizet size;

			u8* End() const
			{
				return start + size;
			}

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
		ArenaBlock block{};
		TArray<Slot> freeSlots{};
		bool pendingSort = false;
		sizet freeSize   = 0;


	public:
		BestFitArena(Arena* parent, const sizet initialSize = 4 * Memory::KB);
		BestFitArena(const sizet initialSize = 4 * Memory::KB) : BestFitArena(nullptr, initialSize)
		{}
		~BestFitArena() override;

		void* Alloc(const sizet size);
		void* Alloc(const sizet size, sizet align);
		bool Realloc(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size);

		const ArenaBlock& GetBlock() const
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

	private:
		i32 FindSmallestSlot(sizet neededSize);
		void ReduceSlot(
		    i32 slotIndex, Slot& slot, u8* const allocationStart, u8* const allocationEnd);
		void AbsorbFreeSpace(u8* const allocationStart, u8* const allocationEnd);

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<BestFitArena>();
		}
	};
#pragma endregion Best Fit Arena

#pragma region Big Best Fit Arena
	class P_API BigBestFitArena : public ChildArena
	{
	public:
		struct AllocationHeader
		{
			u8* end;
		};

		struct P_API Slot
		{
			u32 offset;
			u32 size;

			bool operator==(const Slot& other) const
			{
				return size == other.size;
			}
			auto operator<=>(const Slot& other) const
			{
				return size <=> other.size;
			}
		};

	protected:
		static constexpr sizet minAlignment = sizeof(AllocationHeader);
		// TODO: Support growing multiple blocks
		ArenaBlock block{};
		TArray<Slot> freeSlots{};
		bool pendingSort = false;
		sizet freeSize   = 0;


	public:
		BigBestFitArena(Arena* parent, const sizet initialSize = 1024);
		BigBestFitArena(const sizet initialSize = 1024) : BigBestFitArena(nullptr, initialSize) {}
		~BigBestFitArena() override;

		void* Alloc(const sizet size);
		void* Alloc(const sizet size, sizet alignment);
		bool Realloc(void* ptr, sizet ptrSize, sizet size)
		{
			return false;
		}
		void Free(void* ptr, sizet size);

		const ArenaBlock& GetBlock() const
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
		void ReduceSlot(i32 slotIndex, Slot& slot, u32 allocationStart, u32 allocationEnd);
		void AbsorbFreeSpace(u32 allocationStart, u32 allocationEnd);

		static u32 ToOffset(void* data, void* block);

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<BigBestFitArena>();
		}
	};
#pragma endregion Big Best Fit Arena
}    // namespace p
