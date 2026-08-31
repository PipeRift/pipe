// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Memory/MemoryStats.h"

#include "Pipe/Core/Map.h"
#include "PipeMath.h"
#include "PipeStrings.h"


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
	// MemoryStats::LiveIndex
	// ---------------------------------------------------------------------------

	void MemoryStats::LiveIndex::Grow()
	{
		TArray<u64> oldKeys  = Move(keys);
		TArray<i32> oldNodes = Move(nodes);
		const i32 newCap     = oldKeys.IsEmpty() ? 64 : oldKeys.Size() * 2;

		keys  = TArray<u64>{*arena};
		nodes = TArray<i32>{*arena};
		keys.AddUninitialized(newCap);
		nodes.AddUninitialized(newCap);
		for (i32 i = 0; i < newCap; ++i)
		{
			keys[i]  = 0;
			nodes[i] = Empty;
		}
		mask      = u64(newCap - 1);
		count     = 0;
		tombCount = 0;

		for (i32 i = 0; i < oldKeys.Size(); ++i)
		{
			if (oldNodes[i] >= 0)
			{
				// Insert without grow or duplicates
				const u64 hash = oldKeys[i];
				u64 idx        = hash & mask;
				while (nodes[idx] != Empty)
				{
					idx = (idx + 1) & mask;
				}
				keys[idx]  = hash;
				nodes[idx] = oldNodes[i];
				++count;
			}
		}
	}

	i32* MemoryStats::LiveIndex::Find(u64 hash)
	{
		if (count + tombCount <= 0)
		{
			return nullptr;
		}
		u64 idx = hash & mask;
		while (true)
		{
			const i32 node = nodes[idx];
			if (node == Empty)
			{
				return nullptr;
			}
			if (node != Tombstone && keys[idx] == hash)
			{
				return &nodes[idx];
			}
			idx = (idx + 1) & mask;
		}
	}

	i32* MemoryStats::LiveIndex::FindOrInsert(u64 hash, i32 node)
	{
		// Grow up front when the table is empty or this insert would exceed
		// load. A probe on an empty table would read out of bounds, and
		// growing after a probe would invalidate its result. The rare cost
		// is growing on a find-hit when load is already at the limit.
		if ((count + tombCount + 1) * 4 > i64(keys.Size()) * 3)
		{
			Grow();
		}

		constexpr u64 noTomb = ~u64{0};
		u64 tombstone        = noTomb;
		u64 idx              = hash & mask;
		while (true)
		{
			const i32 n = nodes[idx];
			if (n == Empty)
			{
				break;
			}
			if (n == Tombstone)
			{
				if (tombstone == noTomb)
				{
					tombstone = idx;
				}
			}
			else if (keys[idx] == hash)
			{
				return &nodes[idx];
			}
			idx = (idx + 1) & mask;
		}

		if (tombstone != noTomb)
		{
			idx = tombstone;
			--tombCount;
		}
		keys[idx]  = hash;
		nodes[idx] = node;
		++count;
		return &nodes[idx];
	}

	void MemoryStats::LiveIndex::EraseAt(i32* node)
	{
		*node = Tombstone;
		--count;
		++tombCount;
	}

	void MemoryStats::LiveIndex::Clear()
	{
		keys.Clear();
		nodes.Clear();
		mask      = 0;
		count     = 0;
		tombCount = 0;
	}


	// ---------------------------------------------------------------------------
	// MemoryStats
	// ---------------------------------------------------------------------------

	namespace
	{
		void PrintAllocationError(StringView error, const MemoryStatsEvent* allocation)
		{
			String msg;
			FormatTo(msg, error);
			if (allocation)
			{
				FormatTo(msg, " ({} {})", static_cast<void*>(allocation->GetPtr()),
				    Strings::ParseMemorySize(allocation->GetSize()));
			}
			std::puts(msg.data());
		}
	}    // namespace

	MemoryStats::MemoryStats()
	    : events{GetStatsArena()}
	    , live{GetStatsArena()}
	    , liveIdx{GetStatsArena()}
	    , prevLiveIdx{GetStatsArena()}
	{}

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
			ThreadContext* nextCtx      = c->nextCtx;
			ThreadContext::Chunk* chunk = c->head.load(std::memory_order_relaxed);
			while (chunk)
			{
				ThreadContext::Chunk* next = chunk->next.load(std::memory_order_relaxed);
				chunk->~Chunk();
				p::Free<ThreadContext::Chunk>(GetStatsArena(), chunk, 1);
				chunk = next;
			}
			if (ThreadContext::Chunk* spare = c->spare.load(std::memory_order_relaxed))
			{
				spare->~Chunk();
				p::Free<ThreadContext::Chunk>(GetStatsArena(), spare, 1);
			}
			c->~ThreadContext();
			p::Free<ThreadContext>(GetStatsArena(), c, 1);
			c = nextCtx;
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

	void MemoryStats::PushEvent(const MemoryStatsEvent& ev)
	{
		ThreadContext* ctx = GetOrCreateContext();
		if (ThreadContext::Chunk* chunk = ctx->tail)
		{
			const u32 idx = chunk->writeIdx.load(std::memory_order_relaxed);
			if (idx < ThreadContext::Chunk::capacity)
			{
				chunk->slots[idx] = ev;
				// Release the write so the consumer sees the slot data
				// before the new writeIdx.
				chunk->writeIdx.store(idx + 1, std::memory_order_release);
				return;
			}
		}

		// Cold path: no chunk yet or current chunk is full. Allocate a new
		// chunk, reusing the spare if the consumer left one. The event is
		// written into slot 0 before publishing so the consumer sees a
		// complete slot on first read.
		ThreadContext::Chunk* newC = ctx->spare.exchange(nullptr, std::memory_order_acquire);
		if (!newC)
		{
			newC = p::Alloc<ThreadContext::Chunk>(GetStatsArena(), 1);
		}
		new (newC) ThreadContext::Chunk{};
		newC->slots[0] = ev;
		newC->writeIdx.store(1, std::memory_order_release);
		if (ThreadContext::Chunk* const oldTail = ctx->tail)
		{
			// Publish new chunk via the old chunk's next. Consumer
			// discovers it after we've fully initialized newC.
			oldTail->next.store(newC, std::memory_order_release);
		}
		else
		{
			// First chunk: publish via head.
			ctx->head.store(newC, std::memory_order_release);
		}
		ctx->tail = newC;
	}

	void MemoryStats::Release()
	{
		// Drain all thread buffers and reset state.
		CollectStats();
		used            = 0;
		totalAllocated  = 0;
		collectedEvents = 0;
		events.Clear();
		live.Clear();
		liveIdx.Clear();
		prevLiveIdx.Clear();
	}

	void MemoryStats::CollectStats() const
	{
		// Walk all thread contexts and drain their chunk chains. For each
		// chunk, process all available events, then recycle or free the
		// chunk if the producer has already linked a successor.
		ThreadContext* c = contexts.load(std::memory_order_acquire);
		for (; c != nullptr; c = c->nextCtx)
		{
			ThreadContext::Chunk* chunk = c->head.load(std::memory_order_acquire);
			while (chunk != nullptr)
			{
				const u32 writeIdx = chunk->writeIdx.load(std::memory_order_acquire);
				while (chunk->readIdx < writeIdx)
				{
					const auto& ev = chunk->slots[chunk->readIdx];
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
				// Producer has moved on. Safe to recycle or free this chunk.
				c->head.store(next, std::memory_order_relaxed);
				chunk->~Chunk();
				if (!c->spare.load(std::memory_order_relaxed))
				{
					c->spare.store(chunk, std::memory_order_relaxed);
				}
				else
				{
					p::Free<ThreadContext::Chunk>(GetStatsArena(), chunk, 1);
				}
				chunk = next;
			}
		}


		// --- Incremental classification of drained events ---
		// live[i]: events[i] is an alloc never matched by a free.
		// Events are append-only, so bits computed in previous calls remain
		// valid; only classify events drained since the last call. A free
		// matches the most recent unmatched alloc with the same key (LIFO),
		// mirroring a full reverse scan. Free events are classified by
		// their flag in the event itself.
		live.Resize(events.Size());
		prevLiveIdx.Resize(events.Size());

		for (i32 i = collectedEvents; i < events.Size(); ++i)
		{
			const MemoryStatsEvent& ev = events[i];
			const u64 hash             = GetHash(ev);
			if (ev.IsFree())
			{
				if (i32* nodePtr = liveIdx.Find(hash))
				{
					// Unmark the newest unmatched alloc and pop it off the
					// chain, promoting its predecessor as chain head.
					const i32 node = *nodePtr;
					live.SetFalse(node);
					const i32 prev = prevLiveIdx[node];
					if (prev == NO_INDEX)
					{
						liveIdx.EraseAt(nodePtr);
					}
					else
					{
						*nodePtr = prev;
					}
				}
				// Else a stray free: recorded, nothing to unmark.
			}
			else
			{
				i32* headPtr = liveIdx.FindOrInsert(hash, i);
				if (*headPtr != i)
				{
					prevLiveIdx[i] = *headPtr;
					*headPtr       = i;
				}
				else
				{
					prevLiveIdx[i] = NO_INDEX;
				}
				live.SetTrue(i);
			}
		}
		collectedEvents = events.Size();
	}

	void MemoryStats::CheckLeaks() const
	{
		const i32 numLeaks = live.CountSetBits();
		if (numLeaks <= 0)
		{
			return;
		}

		String errorMsg;
		FormatTo(errorMsg, "{}: {} allocs were not freed!", name ? name : "MemoryStats", numLeaks);

		const i32 shown = Min(64, numLeaks);
		i32 i           = -1;
		i32 printed     = 0;
		while (printed < shown)
		{
			i = live.GetNextSet(i);
			if (i == NO_INDEX)
			{
				break;
			}
			PrintAllocationError("", &events[i]);
			++printed;
		}
		if (numLeaks > shown)
		{
			FormatTo(errorMsg, "\n...\n{} more not shown.", numLeaks - shown);
		}
		std::puts(errorMsg.data());
	}
}    // namespace p
