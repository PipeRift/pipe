// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/Hash.h"
#include "Pipe/Core/Set.h"
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
			return a.ptr == b.ptr;
		}
	};
#pragma pack(pop)

	inline sizet GetHash(const MemoryStatsEvent& ev) noexcept
	{
		return GetHash(ev.GetPtr());
	}
	static_assert(sizeof(MemoryStatsEvent) == 16);


	enum class MemoryStatsErrorType : u8
	{
		None,
		UnknownFree,
		SizeMismatch,      // free(ptr, X) but live has same ptr with size != X
		UnfreedRealloc,    // alloc(ptr, X) but live already has same ptr
	};

	struct P_API MemoryStatsError
	{
		MemoryStatsEvent event;
		MemoryStatsErrorType kind;
	};


	struct P_API MemoryStats
	{
		mutable const char* name = nullptr;

		// When true, leaks are checked when the arena is destroyed.
		// Mutable so it can be flipped through a const GetStats() pointer.
		mutable bool detectLeaks = true;

		// Live allocs keyed by pointer. At most one entry per pointer;
		// duplicate allocs are reported as UnfreedRealloc errors.
		mutable TSet<MemoryStatsEvent> liveAllocations;
		mutable TArray<MemoryStatsError> errors;

		mutable sizet used           = 0;
		mutable sizet totalAllocated = 0;

	private:
		// --- Incremental CollectStats state (consumer thread only) ---
		// Classifier scratch: drained events awaiting classification.
		// CollectStats is not reentrant; owned by the consuming thread.
		mutable TArray<MemoryStatsEvent> pending;
		mutable TArray<MemoryStatsErrorType> pendingErrors;

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
			u32 size         = 0;
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
