// Copyright 2015-2022 Piperift - All rights reserved

#include "ECS/Context.h"


namespace pipe::ECS
{
	Context::Context()
	{
		// TODO: Cache pools
	}

	Context::Context(const Context& other) noexcept
	{
		CopyFrom(other);
	}
	Context::Context(Context&& other) noexcept
	{
		MoveFrom(Move(other));
	}
	Context& Context::operator=(const Context& other) noexcept
	{
		Reset();
		CopyFrom(other);
		return *this;
	}
	Context& Context::operator=(Context&& other) noexcept
	{
		Reset();
		MoveFrom(Move(other));
		return *this;
	}

	Id Context::Create()
	{
		return idRegistry.Create();
	}
	void Context::Create(TSpan<Id> ids)
	{
		idRegistry.Create(ids);
	}

	void Context::Destroy(const Id id)
	{
		idRegistry.Destroy(id);
		for (auto& pool : pools)
		{
			pool.GetPool()->Remove(id);
		}
	}

	void Context::Destroy(TSpan<const Id> ids)
	{
		idRegistry.Destroy(ids);
		for (auto& pool : pools)
		{
			pool.GetPool()->Remove(ids);
		}
	}

	Pool* Context::GetPool(refl::TypeId componentId) const
	{
		const i32 index = pools.FindSortedEqual(PoolInstance{componentId});
		return index != NO_INDEX ? pools[index].GetPool() : nullptr;
	}

	void Context::CopyFrom(const Context& other)
	{
		// Copy entities
		idRegistry = other.idRegistry;

		// Copy component pools. Assume already sorted
		for (const PoolInstance& otherInstance : other.pools)
		{
			PoolInstance instance{otherInstance};
			instance.pool->SetOwnerContext(*this);
			pools.Add(Move(instance));
		}

		// TODO: Copy statics
		// TODO: Cache pools
	}

	void Context::MoveFrom(Context&& other)
	{
		idRegistry = Move(other.idRegistry);
		pools      = Move(other.pools);
		for (auto& instance : pools)
		{
			instance.pool->SetOwnerContext(*this);
		}
		statics = Move(other.statics);

		// TODO: Move statics
		// TODO: Cache pools
	}
}    // namespace pipe::ECS
