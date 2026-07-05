// Copyright 2015-2026 Piperift. All Rights Reserved.

#pragma once

#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/Hash.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Utility.h"
#include "PipeContainers.h"
#include "PipeMath.h"
#include "PipeMemory.h"
#include "PipePlatform.h"

#include <atomic>


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
		MemoryStatsEvent(u8* ptr, sizet size) : ptr{ptr}, size{size} {}
		// Construct with size and flag (for the free ring).
		MemoryStatsEvent(u8* ptr, sizet size, MemoryStatsEventFlags flags)
		    : ptr{ptr}, size{size | *flags}
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


	struct P_API MemoryStats
	{
		const char* name = nullptr;

		// When true, leaks are checked when the arena is destroyed.
		// Mutable so it can be flipped through a const GetStats() pointer.
		mutable bool detectLeaks = true;

		mutable TArray<MemoryStatsEvent> events;
		mutable sizet used           = 0;
		mutable sizet totalAllocated = 0;

		MemoryStats();
		~MemoryStats();

		void Add(void* ptr, sizet size);
		void Remove(void* ptr, sizet size);

		// Empty memory stats. No diagnostics.
		void Release();

		// Update stats so that latest stats and events are reflected
		void CollectStats() const;

		// Check allocation events for leaks. CollectStats needs to be called before.
		void CheckLeaks() const;

	private:
		struct ThreadContext
		{
			// Linked list of chunks. Producer writes to tail; consumer reads
			// from head. The chain is append-only.
			struct Chunk
			{
				static constexpr u32 capacity = 4096;
				std::atomic<Chunk*> next{nullptr};
				MemoryStatsEvent slots[capacity];
				std::atomic<u32> writeIdx{0};
				u32 readIdx = 0;    // consumer-only
			};

			// First chunk with unread events. Producer sets once (release);
			// consumer advances to next chunk (relaxed) as it drains.
			std::atomic<Chunk*> head{nullptr};
			// Producer's current chunk. Consumer does not access.
			Chunk* tail           = nullptr;
			MemoryStats* owner    = nullptr;
			ThreadContext* nextCtx = nullptr;
		};

		mutable std::atomic<ThreadContext*> contexts{nullptr};

		ThreadContext* GetOrCreateContext();

		void PushEvent(void* ptr, sizet size, bool isFree);
	};

	P_API Arena& GetStatsArena();
}    // namespace p
