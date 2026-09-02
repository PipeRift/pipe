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
	    : live{GetStatsArena()}, liveIdx{GetStatsArena()}, pending{GetStatsArena()}
	{}

	MemoryStats::~MemoryStats()
	{
		// Final drain + optional leak diagnostics.
		CollectStats();
		if (detectLeaks)
		{
			CheckLeaks();
		}
		// Free the remaining queue chunks. The producer is no longer running
		// (the MemoryStats is being destroyed), so it's safe to free any
		// remaining chunks.
		ScopedLock guard(lock);
		EventChunk* chunk = firstChunk;
		while (chunk)
		{
			EventChunk* const next = chunk->next;
			chunk->~EventChunk();
			p::Free<EventChunk>(GetStatsArena(), chunk, 1);
			chunk = next;
		}
		if (EventChunk* freed = spareChunk)
		{
			freed->~EventChunk();
			p::Free<EventChunk>(GetStatsArena(), freed, 1);
		}
		firstChunk = nullptr;
		lastChunk  = nullptr;
		spareChunk = nullptr;
	}

	void MemoryStats::PushEvent(const MemoryStatsEvent& ev)
	{
		ScopedLock guard(lock);

		EventChunk* chunk = lastChunk;
		if (!chunk || chunk->size >= EventChunk::capacity) [[unlikely]]
		{
			// Cold path: no chunk yet or current chunk is full. Allocate a new
			// chunk, reusing the spareChunk if the collector left one.
			chunk = spareChunk;
			if (chunk)
			{
				spareChunk = nullptr;
			}
			else
			{
				chunk = p::Alloc<EventChunk>(GetStatsArena(), 1);
			}
			new (chunk) EventChunk{};

			if (lastChunk)
			{
				lastChunk->next = chunk;
			}
			else
			{
				firstChunk = chunk;
			}
			lastChunk = chunk;
		}
		chunk->slots[chunk->size] = ev;
		++chunk->size;
	}

	void MemoryStats::Reset()
	{
		// Drain all thread buffers and reset state.
		CollectStats();
		used           = 0;
		totalAllocated = 0;
		live.Clear();
		liveIdx.Clear();
	}

	void MemoryStats::CollectStats() const
	{
		// Phase 1: drain the shared event queue into a scratch buffer,
		// keeping the lock hold time to just the memcpy.
		pending.Clear();
		{
			ScopedLock guard(lock);

			EventChunk* chunk = firstChunk;
			while (chunk)
			{
				pending.Append(chunk->slots, chunk->size);

				EventChunk* const next = chunk->next;

				chunk->~EventChunk();
				if (!spareChunk)
				{
					spareChunk = chunk;
				}
				else
				{
					p::Free<EventChunk>(GetStatsArena(), chunk, 1);
				}
				chunk = next;
			}
			firstChunk = nullptr;
			lastChunk  = nullptr;
		}

		// Phase 2: classify drained events outside the lock. Alloc events
		// become entries in `live`; matched frees swap-remove them.
		// Nothing else is retained, so memory stays O(live).
		for (const MemoryStatsEvent& ev : pending)
		{
			const u64 hash   = GetHash(ev);
			const sizet size = ev.GetSize();
			if (ev.IsFree())
			{
				if (i32* nodePtr = liveIdx.Find(hash))
				{
					// Swap-remove the matched alloc from the live list,
					// then point its map slot at the moved-in element.
					const i32 p       = *nodePtr;
					const i32 lastIdx = live.Size() - 1;
					if (lastIdx != p)
					{
						live[p] = live[lastIdx];
						if (i32* movedSlot = liveIdx.Find(GetHash(live[p])))
						{
							if (*movedSlot == lastIdx)
							{
								*movedSlot = p;
							}
						}
					}
					live.RemoveLast(1, Shrink::No);
					liveIdx.EraseAt(nodePtr);
					used -= size;
				}
				// Else a stray free: no matching live alloc, ignore.
			}
			else
			{
				live.Add(ev);
				const i32 idx   = live.Size() - 1;
				i32* const slot = liveIdx.FindOrInsert(hash, idx);
				*slot           = idx;
				used += size;
				totalAllocated += size;
			}
		}
	}

	void MemoryStats::CheckLeaks() const
	{
		const i32 numLeaks = live.Size();
		if (numLeaks <= 0)
		{
			return;
		}

		String errorMsg;
		FormatTo(errorMsg, "{}: {} allocs were not freed!", name ? name : "MemoryStats", numLeaks);

		const i32 shown = Min(64, numLeaks);
		i32 printed     = 0;
		for (const auto& ev : live)
		{
			if (printed >= shown)
			{
				break;
			}
			PrintAllocationError("", &ev);
			++printed;
		}
		if (numLeaks > shown)
		{
			FormatTo(errorMsg, "\n...\n{} more not shown.", numLeaks - shown);
		}
		std::puts(errorMsg.data());
	}
}    // namespace p
