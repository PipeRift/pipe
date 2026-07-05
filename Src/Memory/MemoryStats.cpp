// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Memory/MemoryStats.h"

#include "Pipe/Core/Set.h"
#include "Pipe/Core/String.h"
#include "PipeMath.h"


namespace p
{
	// Non-tracking arena. All MemoryStats storage goes through here so
	// that the stats system never re-enters itself through an Alloc hook.
	class P_API MemoryStatsArena : public Arena
	{
	public:
		MemoryStatsArena()
		{
			Interface<MemoryStatsArena>();
		}

		inline void* Alloc(const sizet size)
		{
			return std::malloc(size);
		}
		inline void* Alloc(const sizet size, const sizet align)
		{
			return std::malloc(size);
		}
		inline bool Realloc(void* ptr, const sizet ptrSize, const sizet size)
		{
			return false;
		}
		inline void Free(void* ptr, sizet size)
		{
			std::free(ptr);
		}

	protected:
		TypeId ProvideTypeId() const override
		{
			return p::GetTypeId<MemoryStatsArena>();
		}
	};

	Arena& GetStatsArena()
	{
		static MemoryStatsArena arena;
		return arena;
	}


	// ---------------------------------------------------------------------------
	// MemoryStats
	// ---------------------------------------------------------------------------

	namespace
	{
		void PrintAllocationError(StringView error, const MemoryStatsEvent* allocation)
		{
			String msg;
			Strings::FormatTo(msg, error);
			if (allocation)
			{
				Strings::FormatTo(msg, " ({} {})", static_cast<void*>(allocation->GetPtr()),
				    Strings::ParseMemorySize(allocation->GetSize()));
			}
			std::puts(msg.data());
		}
	}    // namespace

	MemoryStats::MemoryStats() : events{GetStatsArena()} {}

	MemoryStats::~MemoryStats()
	{
		// Final drain + optional leak diagnostics.
		CollectStats();
		if (detectLeaks)
		{
			CheckLeaks();
		}
		// Delete all thread contexts. The producer is no longer running
		// (the MemoryStats is being destroyed), so it's safe to free any
		// remaining chunks.
		ThreadContext* c = contexts.exchange(nullptr, std::memory_order_acq_rel);
		while (c)
		{
			ThreadContext* next = c->nextCtx;
			if (ThreadContext::Chunk* chunk = c->head.load(std::memory_order_relaxed))
			{
				chunk->~Chunk();
				p::Free<ThreadContext::Chunk>(GetStatsArena(), chunk, 1);
			}
			c->~ThreadContext();
			p::Free<ThreadContext>(GetStatsArena(), c, 1);
			c = next;
		}
	}

	MemoryStats::ThreadContext* MemoryStats::GetOrCreateContext()
	{
		// Per-thread, per-MemoryStats context. The thread_local cache holds
		// the most recently used context; we replace it if the owner changed.
		thread_local ThreadContext* ctx = nullptr;
		if (!ctx || ctx->owner != this)
		{
			ctx = p::Alloc<ThreadContext>(GetStatsArena(), 1);
			new (ctx) ThreadContext{};
			ctx->owner = this;
			// Link into the global list. Append-only, so no synchronization
			// needed with the consumer beyond the CAS.
			ThreadContext* old = contexts.load(std::memory_order_relaxed);
			do
			{
				ctx->nextCtx = old;
			} while (!contexts.compare_exchange_weak(
			    old, ctx, std::memory_order_release, std::memory_order_relaxed));
		}
		return ctx;
	}

	void MemoryStats::PushEvent(void* ptr, sizet size, bool isFree)
	{
		auto* ctx               = GetOrCreateContext();
		ThreadContext::Chunk* c = ctx->tail;
		if (!c || c->writeIdx.load(std::memory_order_relaxed) >= ThreadContext::Chunk::capacity)
		{
			// Allocate a new chunk. Initialize it with the event already
			// written so the consumer sees a complete slot on first read.
			ThreadContext::Chunk* newC = p::Alloc<ThreadContext::Chunk>(GetStatsArena(), 1);
			new (newC) ThreadContext::Chunk{};
			if (isFree)
			{
				newC->slots[0] = {static_cast<u8*>(ptr), size, MemoryStatsEventFlags::IsFree};
			}
			else
			{
				newC->slots[0] = {static_cast<u8*>(ptr), size};
			}
			newC->writeIdx.store(1, std::memory_order_release);
			if (c)
			{
				// Publish new chunk via the old chunk's next. Consumer
				// discovers it after we've fully initialized newC.
				c->next.store(newC, std::memory_order_release);
			}
			else
			{
				// First chunk: publish via head.
				ctx->head.store(newC, std::memory_order_release);
			}
			ctx->tail = newC;
			return;
		}
		const u32 idx = c->writeIdx.load(std::memory_order_relaxed);
		if (isFree)
		{
			c->slots[idx] = {static_cast<u8*>(ptr), size, MemoryStatsEventFlags::IsFree};
		}
		else
		{
			c->slots[idx] = {static_cast<u8*>(ptr), size};
		}
		// Release the write so the consumer sees the slot data before the
		// new writeIdx.
		c->writeIdx.store(idx + 1, std::memory_order_release);
	}

	void MemoryStats::Add(void* ptr, sizet size)
	{
		PushEvent(ptr, size, false);
	}

	void MemoryStats::Remove(void* ptr, sizet size)
	{
		if (!ptr)
		{
			return;
		}
		PushEvent(ptr, size, true);
	}

	void MemoryStats::Release()
	{
		// Drain all thread buffers and reset state.
		CollectStats();
		used           = 0;
		totalAllocated = 0;
		events.Clear();
	}

	void MemoryStats::CollectStats() const
	{
		// Walk all thread contexts and drain their chunk chains. For each
		// chunk, process all available events, then free the chunk if the
		// producer has already linked a successor.
		ThreadContext* c = contexts.load(std::memory_order_acquire);
		for (; c != nullptr; c = c->nextCtx)
		{
			ThreadContext::Chunk* chunk = c->head.load(std::memory_order_acquire);
			while (chunk != nullptr)
			{
				const u32 writeIdx = chunk->writeIdx.load(std::memory_order_acquire);
				while (chunk->readIdx < writeIdx)
				{
					const MemoryStatsEvent& ev = chunk->slots[chunk->readIdx];
					if (ev.IsFree())
					{
						used -= ev.GetSize();
						totalAllocated -= ev.GetSize();
					}
					else
					{
						used += ev.GetSize();
						totalAllocated += ev.GetSize();
					}
					events.Add(ev);
					++chunk->readIdx;
				}
				ThreadContext::Chunk* next = chunk->next.load(std::memory_order_acquire);
				if (next == nullptr)
				{
					// Producer hasn't allocated a successor yet. Stop.
					break;
				}
				// Producer has moved on. Safe to free this chunk.
				c->head.store(next, std::memory_order_relaxed);
				chunk->~Chunk();
				p::Free<ThreadContext::Chunk>(GetStatsArena(), chunk, 1);
				chunk = next;
			}
		}
	}

	void MemoryStats::CheckLeaks() const
	{
		TSet<MemoryStatsEvent> live{GetStatsArena()};
		for (const auto& ev : events)
		{
			if (ev.IsFree())
			{
				live.Remove(ev);
			}
			else
			{
				live.Insert(ev);
			}
		}

		if (live.IsEmpty())
		{
			return;
		}

		String errorMsg;
		Strings::FormatTo(errorMsg, "{}: {} allocs were not freed!", name, live.Size());

		const i32 shown = Min(64, live.Size());
		i32 i           = 0;
		for (const auto& ev : live)
		{
			if (i >= shown)
			{
				break;
			}
			PrintAllocationError("", &ev);
			++i;
		}
		if (live.Size() > shown)
		{
			Strings::FormatTo(errorMsg, "\n...\n{} more not shown.", live.Size() - shown);
		}
		std::puts(errorMsg.data());
	}
}    // namespace p
