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
	    : liveAllocations{GetStatsArena()}
	    , errors{GetStatsArena()}
	    , pending{GetStatsArena()}
	    , pendingErrors{GetStatsArena()}
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
		liveAllocations.Clear();
		errors.Clear();
	}

	void MemoryStats::CollectStats() const
	{
		// Phase 1: drain the shared event queue into a scratch buffer,
		// keeping the lock hold time to just the memcpy.
		pending.Clear();
		EventChunk* chunk;
		{    // Guard detaches the chunk list; producers can no longer reach it.
			ScopedLock guard(lock);
			chunk      = firstChunk;
			firstChunk = nullptr;
			lastChunk  = nullptr;
			if (chunk && !spareChunk)    // Spare chunk still needs lock and draining
			{
				pending.Append(chunk->slots, chunk->size);
				spareChunk = chunk;
				chunk      = chunk->next;
				spareChunk->~EventChunk();
			}
		}

		// Drain safely events from all detached chunks
		while (chunk)
		{
			pending.Append(chunk->slots, chunk->size);

			EventChunk* const next = chunk->next;
			chunk->~EventChunk();
			p::Free<EventChunk>(GetStatsArena(), chunk, 1);
			chunk = next;
		}
		pendingErrors.Resize(pending.Size(), MemoryStatsErrorType::None);

		// Iterate events to track live allocations and errors
		for (i32 i = 0; i < pending.Size(); ++i)
		{
			const MemoryStatsEvent& ev = pending[i];
			if (ev.IsFree())
			{
				if (MemoryStatsEvent* liveEv = liveAllocations.Find(ev))
				{
					if (liveEv->GetSize() != ev.GetSize())
					{
						pendingErrors[i] = MemoryStatsErrorType::SizeMismatch;
					}
					else
					{
						liveAllocations.Remove(ev);
					}
				}
				else
				{
					pendingErrors[i] = MemoryStatsErrorType::UnknownFree;
				}
			}
			else
			{
				if (liveAllocations.Contains(ev))
				{
					pendingErrors[i] = MemoryStatsErrorType::UnfreedRealloc;
				}
				else
				{
					liveAllocations.Insert(ev);
				}
			}
		}

		// Record errors and remove events so that stats are calculated correctly.\
		// (Order of events is no longer needed)
		for (i32 i = 0; i < pendingErrors.Size(); ++i)
		{
			if (pendingErrors[i] != MemoryStatsErrorType::None)
			{
				errors.Add({pending[i], pendingErrors[i]});

				pending.RemoveAtSwapUnsafe(i);
				pendingErrors.RemoveAtSwapUnsafe(i);
				--i;
			}
		}

		// Record stats
		for (i32 i = 0; i < pending.Size(); ++i)
		{
			const MemoryStatsEvent& ev = pending[i];
			const sizet size           = ev.GetSize();
			if (ev.IsFree())
			{
				used -= size;
			}
			else
			{
				used += size;
				totalAllocated += size;
			}
		}
	}

	void MemoryStats::CheckLeaks() const
	{
		const i32 numLeaks = liveAllocations.Size();
		if (numLeaks <= 0)
		{
			return;
		}

		String errorMsg;
		FormatTo(errorMsg, "{}: {} allocs were not freed!", name ? name : "MemoryStats", numLeaks);

		const i32 shown = Min(64, numLeaks);
		i32 printed     = 0;
		for (const auto& ev : liveAllocations)
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
