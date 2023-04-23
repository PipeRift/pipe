// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Profiler.h"
#include "Pipe/ECS/Access.h"
#include "Pipe/ECS/Context.h"


namespace p::ecs
{
	/** Remove ids containing a component from 'ids'. Does not guarantee order. */
	PIPE_API void ExcludeIf(const BasePool* pool, TArray<Id>& ids, const bool shouldShrink = true);

	/** Remove ids containing a component from 'ids'. Guarantees order. */
	PIPE_API void ExcludeIfStable(
	    const BasePool* pool, TArray<Id>& ids, const bool shouldShrink = true);

	/** Remove ids NOT containing a component from 'ids'. Does not guarantee order. */
	PIPE_API void ExcludeIfNot(
	    const BasePool* pool, TArray<Id>& ids, const bool shouldShrink = true);

	/** Remove ids NOT containing a component from 'ids'. Guarantees order. */
	PIPE_API void ExcludeIfNotStable(
	    const BasePool* pool, TArray<Id>& ids, const bool shouldShrink = true);


	/** Find ids containing a component from a list 'source' into 'results'. */
	PIPE_API void GetIf(const BasePool* pool, const TSpan<Id>& source, TArray<Id>& results);
	PIPE_API void GetIf(
	    const TArray<const BasePool*>& pools, const TSpan<Id>& source, TArray<Id>& results);

	/** Find ids NOT containing a component from a list 'source' into 'results'. */
	PIPE_API void GetIfNot(const BasePool* pool, const TSpan<Id>& source, TArray<Id>& results);


	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Does not guarantee order.
	 */
	PIPE_API void ExtractIf(const BasePool* pool, TArray<Id>& source, TArray<Id>& results,
	    const bool shouldShrink = true);

	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Guarantees order.
	 */
	PIPE_API void ExtractIfStable(const BasePool* pool, TArray<Id>& source, TArray<Id>& results,
	    const bool shouldShrink = true);

	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Does not guarantee order.
	 */
	PIPE_API void ExtractIfNot(const BasePool* pool, TArray<Id>& source, TArray<Id>& results,
	    const bool shouldShrink = true);

	/**
	 * Find and remove ids not containing a component from list 'source' into 'results'.
	 * Guarantees order.
	 */
	PIPE_API void ExtractIfNotStable(const BasePool* pool, TArray<Id>& source, TArray<Id>& results,
	    const bool shouldShrink = true);


	/** Find all ids containing all of the components */
	PIPE_API void ListAll(TArray<const BasePool*> pools, TArray<Id>& ids);

	/** Find all ids containing any of the components. Includes possible duplicates */
	PIPE_API void ListAny(const TArray<const BasePool*>& pools, TArray<Id>& ids);

	/** Find all ids containing any of the components. Prevents duplicates */
	PIPE_API void ListAnyUnique(const TArray<const BasePool*>& pools, TArray<Id>& ids);


	// Templated API

	/**
	 * Remove ids containing a component from 'ids'. Does not guarantee order.
	 *
	 * @param access from where to access pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see ExcludeIfStable(), ExcludeIfNot()
	 */
	template<typename C, typename AccessType>
	void ExcludeIf(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		ExcludeIf(&access.template AssurePool<const C>(), ids, shouldShrink);
	}
	template<typename... C, typename AccessType>
	void ExcludeIf(const AccessType& access, TArray<Id>& ids,
	    const bool shouldShrink = true) requires(sizeof...(C) > 1)
	{
		(ExcludeIf<C>(access, ids, shouldShrink), ...);
	}

	template<typename Predicate>
	void ExcludeIf(TArray<Id>& ids, Predicate predicate, const bool shouldShrink = true)
	{
		ZoneScoped;
		for (i32 i = ids.Size() - 1; i >= 0; --i)
		{
			if (predicate(ids[i]))
			{
				ids.RemoveAtSwapUnsafe(i);
			}
		}
		if (shouldShrink)
		{
			ids.Shrink();
		}
	}

	/**
	 * Remove ids containing a component from 'ids'. Guarantees order.
	 *
	 * @param access from where to access pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see ExcludeIf(), ExcludeIfNotStable()
	 */
	template<typename C, typename AccessType>
	void ExcludeIfStable(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		ExcludeIfStable(&access.template AssurePool<const C>(), ids, shouldShrink);
	}
	template<typename... C, typename AccessType>
	void ExcludeIfStable(const AccessType& access, TArray<Id>& ids,
	    const bool shouldShrink = true) requires(sizeof...(C) > 1)
	{
		(ExcludeIfStable<C>(access, ids, shouldShrink), ...);
	}

	/**
	 * Remove ids NOT containing a component from 'ids'. Does not guarantee order.
	 *
	 * @param access from where to access pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see ExcludeIfNotStable(), ExcludeIf()
	 */
	template<typename C, typename AccessType>
	void ExcludeIfNot(const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		ExcludeIfNot(&access.template AssurePool<const C>(), ids, shouldShrink);
	}

	template<typename... C, typename AccessType>
	void ExcludeIfNot(const AccessType& access, TArray<Id>& ids,
	    const bool shouldShrink = true) requires(sizeof...(C) > 1)
	{
		(ExcludeIfNot<C>(access, ids, shouldShrink), ...);
	}

	/**
	 * Remove ids NOT containing a component from 'ids'. Guarantees order.
	 *
	 * @param access from where to access pools
	 * @param ids array that will be modified
	 * @param shouldShrink if true, the ids array will be shrink at the end
	 * @see ExcludeIfNot(), ExcludeIfStable()
	 */
	template<typename C, typename AccessType>
	void ExcludeIfNotStable(
	    const AccessType& access, TArray<Id>& ids, const bool shouldShrink = true)
	{
		ExcludeIfNotStable(&access.template AssurePool<const C>(), ids, shouldShrink);
	}
	template<typename... C, typename AccessType>
	void ExcludeIfNotStable(const AccessType& access, TArray<Id>& ids,
	    const bool shouldShrink = true) requires(sizeof...(C) > 1)
	{
		(ExcludeIfNotStable<C>(access, ids, shouldShrink), ...);
	}


	/** Find ids containing a component from a list 'source' into 'results'. */
	template<typename C, typename AccessType>
	void GetIf(const AccessType& access, const TSpan<Id>& source, TArray<Id>& results)
	{
		GetIf(&access.template AssurePool<const C>(), source, results);
	}

	template<typename... C, typename AccessType>
	void GetIf(const AccessType& access, const TSpan<Id>& source, TArray<Id>& results) requires(
	    sizeof...(C) > 1)
	{
		GetIf({&access.template AssurePool<const C>()...}, source, results);
	}

	template<typename... C, typename AccessType>
	TArray<Id> GetIf(const AccessType& access, const TSpan<Id>& source)
	{
		TArray<Id> results;
		GetIf<C...>(access, source, results);
		return Move(results);
	}

	/** Find ids NOT containing a component from a list 'source' into 'results'. */
	template<typename C, typename AccessType>
	void GetIfNot(const AccessType& access, const TArray<Id>& source, TArray<Id>& results)
	{
		GetIfNot(&access.template AssurePool<const C>(), source, results);
	}
	template<typename C, typename AccessType>
	TArray<Id> GetIfNot(const AccessType& access, const TArray<Id>& source)
	{
		TArray<Id> results;
		GetIfNot<C>(access, source, results);
		return Move(results);
	}


	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Does not guarantee order.
	 */
	template<typename C, typename AccessType>
	void ExtractIf(const AccessType& access, TArray<Id>& source, TArray<Id>& results,
	    const bool shouldShrink = true)
	{
		ExtractIf(&access.template AssurePool<const C>(), source, results);
	}
	template<typename C, typename AccessType>
	TArray<Id> ExtractIf(
	    const AccessType& access, TArray<Id>& source, const bool shouldShrink = true)
	{
		TArray<Id> results;
		ExtractIf<C>(access, source, results);
		return Move(results);
	}

	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Guarantees order.
	 */
	template<typename C, typename AccessType>
	void ExtractIfStable(const AccessType& access, TArray<Id>& source, TArray<Id>& results,
	    const bool shouldShrink = true)
	{
		ExtractIfStable(&access.template AssurePool<const C>(), source, results);
	}
	template<typename C, typename AccessType>
	TArray<Id> ExtractIfStable(
	    const AccessType& access, TArray<Id>& source, const bool shouldShrink = true)
	{
		TArray<Id> results;
		ExtractIfStable<C>(access, source, results);
		return Move(results);
	}

	/**
	 * Find and remove ids containing a component from list 'source' into 'results'.
	 * Does not guarantee order.
	 */
	template<typename C, typename AccessType>
	void ExtractIfNot(const AccessType& access, TArray<Id>& source, TArray<Id>& results,
	    const bool shouldShrink = true)
	{
		ExtractIfNot(&access.template AssurePool<const C>(), source, results);
	}
	template<typename C, typename AccessType>
	TArray<Id> ExtractIfNot(
	    const AccessType& access, TArray<Id>& source, const bool shouldShrink = true)
	{
		TArray<Id> results;
		ExtractIfNot<C>(access, source, results);
		return Move(results);
	}

	/**
	 * Find and remove ids not containing a component from list 'source' into 'results'.
	 * Guarantees order.
	 */
	template<typename C, typename AccessType>
	void ExtractIfNotStable(const AccessType& access, TArray<Id>& source, TArray<Id>& results,
	    const bool shouldShrink = true)
	{
		ExtractIfNotStable(&access.template AssurePool<const C>(), source, results);
	}
	template<typename C, typename AccessType>
	TArray<Id> ExtractIfNotStable(
	    const AccessType& access, TArray<Id>& source, const bool shouldShrink = true)
	{
		TArray<Id> results;
		ExtractIfNotStable<C>(access, source, results);
		return Move(results);
	}


	/**
	 * Find all ids containing all of the components
	 *
	 * @param access from where to access pools
	 * @param ids array where matching ids will be added
	 * @see ListAny()
	 */
	template<typename... C, typename AccessType>
	void ListAll(const AccessType& access, TArray<Id>& ids)
	{
		ListAll({&access.template AssurePool<const C>()...}, ids);
	}


	/**
	 * Find all ids containing all of the components
	 *
	 * @param access from where to access pools
	 * @return ids array with matching ids
	 * @see ListAny()
	 */
	template<typename... C, typename AccessType>
	TArray<Id> ListAll(const AccessType& access)
	{
		TArray<Id> ids;
		ListAll<C...>(access, ids);
		return Move(ids);
	}

	/**
	 * Find all ids containing any of the components.
	 * Includes possible duplicates
	 *
	 * @param access from where to access pools
	 * @param ids array where matching ids will be added
	 * @see ListAll()
	 */
	template<typename... C, typename AccessType>
	void ListAny(const AccessType& access, TArray<Id>& ids)
	{
		ListAny({&access.template AssurePool<const C>()...}, ids);
	}

	/**
	 * Find all ids containing any of the components.
	 * Prevents duplicates
	 *
	 * @param access from where to access pools
	 * @param ids array where matching ids will be added
	 * @see ListAnyUnique()
	 */
	template<typename... C, typename AccessType>
	void ListAnyUnique(const AccessType& access, TArray<Id>& ids)
	{
		ListAnyUnique({&access.template AssurePool<const C>()...}, ids);
	}

	/**
	 * Find all ids containing any of the components.
	 * Includes possible duplicates
	 *
	 * @param access from where to access pools
	 * @return ids array with matching ids
	 * @see ListAll()
	 */
	template<typename... C, typename AccessType>
	TArray<Id> ListAny(const AccessType& access)
	{
		TArray<Id> ids;
		ListAny<C...>(access, ids);
		return Move(ids);
	}

	/**
	 * Find all ids containing any of the components.
	 * Prevents duplicates
	 *
	 * @param access from where to access pools
	 * @return ids array with matching ids
	 * @see ListAny()
	 */
	template<typename... C, typename AccessType>
	TArray<Id> ListAnyUnique(const AccessType& access)
	{
		TArray<Id> ids;
		ListAnyUnique<C...>(access, ids);
		return Move(ids);
	}

	template<typename C, typename AccessType>
	Id GetFirst(const AccessType& access)
	{
		const BasePool* pool = access.template GetPool<const C>();
		return (pool && pool->Size() > 0) ? *pool->begin() : ecs::NoId;
	}
}    // namespace p::ecs
