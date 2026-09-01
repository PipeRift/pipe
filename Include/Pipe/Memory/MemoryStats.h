// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/Hash.h"
#include "Pipe/Core/SpinLock.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Utility.h"
#include "PipeContainers.h"
#include "PipeMath.h"
#include "PipeMemory.h"
#include "PipePlatform.h"


namespace p
{
	enum class MemoryStatsEventFlags : sizet
	{
		IsFree = 1ull << ((sizeof(sizet) - 1) * 8),
		Mask   = 0xFFull << ((sizeof(sizet) - 1) * 8)
	};
	P_DEFINE_FLAG_OPERATORS(MemoryStatsEventFlags);


	// 16 B event. Size in low 7 bytes; high byte holds 8 flag bits.
#pragma pack(push, 1)
	struct P_API MemoryStatsEvent
	{
	protected:
		u8* ptr;
		sizet size;

	public:
		MemoryStatsEvent() = default;
		MemoryStatsEvent(void* ptr, sizet size) : ptr{static_cast<u8*>(ptr)}, size{size} {}
		// Construct with size and flag (for the free ring).
		MemoryStatsEvent(void* ptr, sizet size, MemoryStatsEventFlags flags)
		    : ptr{static_cast<u8*>(ptr)}, size{size | *flags}
		{}

		u8* GetPtr() const
		{
			return ptr;
		}
		bool IsValid() const
		{
			return ptr != nullptr;
		}
		sizet GetSize() const
		{
			return size & ~*MemoryStatsEventFlags::Mask;
		}
		MemoryStatsEventFlags GetFlags() const
		{
			return MemoryStatsEventFlags(size & *MemoryStatsEventFlags::Mask);
		}
		bool IsFree() const
		{
			return HasFlag(size, MemoryStatsEventFlags::IsFree);
		}

		friend bool operator==(const MemoryStatsEvent& a, const MemoryStatsEvent& b) noexcept
		{
			return a.ptr == b.ptr && a.GetSize() == b.GetSize();
		}
	};
#pragma pack(pop)

	inline sizet GetHash(const MemoryStatsEvent& ev) noexcept
	{
		return HashCombine(GetHash(ev.GetPtr()), ev.GetSize());
	}
	static_assert(sizeof(MemoryStatsEvent) == 16);


	struct P_API MemoryStats
	{
		mutable const char* name = nullptr;

		// When true, leaks are checked when the arena is destroyed.
		// Mutable so it can be flipped through a const GetStats() pointer.
		mutable bool detectLeaks = true;

		mutable TArray<MemoryStatsEvent> events;

		// Bit i set when events[i] is an allocation that, by the last
		// CollectStats, had not been matched by a corresponding free.
		mutable BitArray live;

		mutable sizet used           = 0;
		mutable sizet totalAllocated = 0;

	private:
		// Open-addressed linear-probe map from event hash to the newest
		// unmatched alloc event index for that hash. Keys are pre-mixed
		// hashes (from GetHash), indexed directly without re-hashing.
		// No per-insert allocation; grows at 75% load.
		class LiveIndex
		{
			static constexpr i32 Empty     = -1;
			static constexpr i32 Tombstone = -2;

			Arena* arena = nullptr;
			TArray<u64> keys;
			// Parallel to keys: the node index, or Empty/Tombstone.
			TArray<i32> nodes;
			u64 mask      = 0;
			i32 count     = 0;
			i32 tombCount = 0;

			void Grow();

		public:
			explicit LiveIndex(Arena& inArena) : arena{&inArena}, keys{inArena}, nodes{inArena} {}

			i32* Find(u64 hash);
			i32* FindOrInsert(u64 hash, i32 node);
			void EraseAt(i32* node);
			void Clear();
		};

		// --- Incremental CollectStats state (consumer thread only) ---
		// Newest unmatched alloc index per event key. Chains are
		// intrusively linked through prevLiveIdx, newest first.
		mutable LiveIndex liveIdx;
		// For each alloc event index, the previous unmatched alloc index
		// sharing the same key (NO_INDEX if none). Consumed on free.
		mutable TArray<i32> prevLiveIdx;

	public:

		MemoryStats();
		~MemoryStats();

		// Tracks an allocation. Writes one 16B event.
		inline void Add(void* ptr, sizet size)
		{
			PushEvent(MemoryStatsEvent{ptr, size});
		}

		// Tracks a free. Ignored for null pointers.
		inline void Remove(void* ptr, sizet size)
		{
			if (ptr)
			{
				PushEvent(MemoryStatsEvent{ptr, size, MemoryStatsEventFlags::IsFree});
			}
		}

		// Empty memory stats. No diagnostics.
		void Reset();

		// Update stats so that latest stats and events are reflected
		void CollectStats() const;

		// Check allocation events for leaks. CollectStats needs to be called before.
		void CheckLeaks() const;

	private:
		// Fixed-size event block. Chunks form a single shared queue guarded
		// by `lock`: all producers append through `tail`, the collector
		// drains from `head` and recycles chunks via `spare`.
		struct EventChunk
		{
			static constexpr u32 capacity = 1024;

			EventChunk* next = nullptr;
			u32 size    = 0;
			MemoryStatsEvent slots[capacity];
		};

		// Queue head/tail and a single recycled chunk. All access is
		// guarded by `lock` (producers on append, collector on drain).
		mutable SpinLock lock;
		mutable EventChunk* firstChunk = nullptr;
		mutable EventChunk* lastChunk  = nullptr;
		mutable EventChunk* spareChunk = nullptr;

		void PushEvent(const MemoryStatsEvent& ev);
	};

	P_API Arena& GetStatsArena();
}    // namespace p
