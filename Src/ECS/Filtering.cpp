// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/ECS/Filtering.h"

#include "Pipe/Core/Set.h"


namespace p::ecs
{
	void ExcludeIf(const BasePool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ZoneScoped;
		for (i32 i = ids.Size() - 1; i >= 0; --i)
		{
			if (pool->Has(ids[i]))
			{
				ids.RemoveAtSwapUnsafe(i);
			}
		}
		if (shouldShrink)
		{
			ids.Shrink();
		}
	}

	void ExcludeIfStable(const BasePool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ZoneScoped;
		ids.RemoveIf(
		    [pool](Id id) {
			return pool->Has(id);
		    },
		    shouldShrink);
	}

	void ExcludeIfNot(const BasePool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ZoneScoped;
		for (i32 i = ids.Size() - 1; i >= 0; --i)
		{
			if (!pool->Has(ids[i]))
			{
				ids.RemoveAtSwapUnsafe(i);
			}
		}
		if (shouldShrink)
		{
			ids.Shrink();
		}
	}

	void ExcludeIfNotStable(const BasePool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ZoneScoped;
		ids.RemoveIf(
		    [pool](Id id) {
			return !pool->Has(id);
		    },
		    shouldShrink);
	}

	void GetIf(const BasePool* pool, const TSpan<Id>& source, TArray<Id>& results)
	{
		ZoneScoped;
		if (pool)
		{
			results.ReserveMore(math::Min(i32(pool->Size()), source.Size()));
			for (ecs::Id id : source)
			{
				if (pool->Has(id))
				{
					results.Add(id);
				}
			}
		}
	}
	void GetIf(const TArray<const BasePool*>& pools, const TSpan<Id>& source, TArray<Id>& results)
	{
		GetIf(pools.First(), source, results);
		for (i32 i = 1; i < pools.Size(); ++i)
		{
			ExcludeIfNot(pools[i], results, false);
		}
	}

	void GetIfNot(const BasePool* pool, const TSpan<Id>& source, TArray<Id>& results)
	{
		ZoneScoped;
		if (pool)
		{
			results.ReserveMore(source.Size());
			for (ecs::Id id : source)
			{
				if (!pool->Has(id))
				{
					results.Add(id);
				}
			}
		}
		else
		{
			// No pool means no id has the component
			results.Append(source);
		}
	}

	void ExtractIf(
	    const BasePool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		ZoneScoped;
		results.ReserveMore(math::Min(i32(pool->Size()), source.Size()));
		for (i32 i = source.Size() - 1; i >= 0; --i)
		{
			const ecs::Id id = source[i];
			if (pool->Has(id))
			{
				source.RemoveAtSwapUnsafe(i);
				results.Add(id);
			}
		}
		if (shouldShrink)
		{
			source.Shrink();
		}
	}

	void ExtractIfStable(
	    const BasePool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		ZoneScoped;
		results.ReserveMore(math::Min(i32(pool->Size()), source.Size()));
		source.RemoveIf(
		    [pool, &results](Id id) {
			if (pool->Has(id))
			{
				results.Add(id);
				return true;
			}
			return false;
		    },
		    shouldShrink);
	}

	void ExtractIfNot(
	    const BasePool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		ZoneScoped;
		results.ReserveMore(source.Size());
		for (i32 i = source.Size() - 1; i >= 0; --i)
		{
			const ecs::Id id = source[i];
			if (!pool->Has(id))
			{
				source.RemoveAtSwapUnsafe(i);
				results.Add(id);
			}
		}
		if (shouldShrink)
		{
			source.Shrink();
		}
	}

	void ExtractIfNotStable(
	    const BasePool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		ZoneScoped;
		results.ReserveMore(math::Min(i32(pool->Size()), source.Size()));
		source.RemoveIf(
		    [pool, &results](Id id) {
			if (!pool->Has(id))
			{
				results.Add(id);
				return true;
			}
			return false;
		    },
		    shouldShrink);
	}

	void ListAll(TArray<const BasePool*> pools, TArray<Id>& ids)
	{
		ZoneScoped;
		for (const BasePool* pool : pools)
		{
			if (!EnsureMsg(pool,
			        "One of the pools is null. Is the access missing one or more of the specified "
			        "components?"))
			{
				return;
			}
		}

		const i32 smallestIdx        = GetSmallestPool(pools);
		const BasePool* iterablePool = pools[smallestIdx];
		pools.RemoveAtSwap(smallestIdx);

		ids.Clear(false);
		ids.Reserve(iterablePool->Size());
		for (ecs::Id id : *iterablePool)
		{
			if (!IsNone(id)) [[likely]]
			{
				ids.Add(id);
			}
		}
		// Faster but doesn't exclude invalid ids.
		// ids.Append(iterablePool->begin(), iterablePool->end());

		for (const BasePool* pool : pools)
		{
			ExcludeIfNot(pool, ids, false);
		}
	}

	void ListAny(const TArray<const BasePool*>& pools, TArray<Id>& ids)
	{
		ZoneScoped;
		for (const BasePool* pool : pools)
		{
			if (!EnsureMsg(pool,
			        "One of the pools is null. Is the access missing one or more of the specified "
			        "components?"))
			{
				return;
			}
		}

		ids.Clear();
		for (const BasePool* pool : pools)
		{
			ids.Append(pool->begin(), pool->end());
		}
	}

	void ListAnyUnique(const TArray<const BasePool*>& pools, TArray<Id>& ids)
	{
		ZoneScoped;
		i32 maxPossibleSize = 0;
		for (const BasePool* pool : pools)
		{
			if (!EnsureMsg(pool,
			        "One of the pools is null. Is the access missing one or more of the specified "
			        "components?"))
			{
				return;
			}

			maxPossibleSize += pool->Size();
		}

		TSet<Id> idsSet;
		idsSet.Reserve(maxPossibleSize);
		for (const BasePool* pool : pools)
		{
			for (Id id : *pool)
			{
				idsSet.Insert(id);
			}
		}

		ids.Clear();
		ids.Append(idsSet.begin(), idsSet.end());
	}
}    // namespace p::ecs
