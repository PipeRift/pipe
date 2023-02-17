// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/ECS/Context.h"


namespace p::ecs
{
	Context::Context() {}

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

	void* Context::AddDefaulted(TypeId typeId, Id id)
	{
		BasePool* pool = GetPool(typeId);
		return pool ? pool->AddDefaulted(id) : nullptr;
	}

	BasePool* Context::GetPool(TypeId componentId) const
	{
		const i32 index = pools.FindSortedEqual(PoolInstance{componentId, {}});
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

	bool Context::IsValid(Id id) const
	{
		return idRegistry.IsValid(id);
	}

	bool Context::IsOrphan(const Id id) const
	{
		for (const auto& instance : pools)
		{
			if (instance.GetPool()->Has(id))
			{
				return false;
			}
		}
		return true;
	}

	void* Context::TryGetStatic(TypeId typeId)
	{
		const i32 index = statics.FindSortedEqual<TypeId, SortLessStatics>(typeId);
		return index != NO_INDEX ? statics[index].Get() : nullptr;
	}
	const void* Context::TryGetStatic(TypeId typeId) const
	{
		const i32 index = statics.FindSortedEqual<TypeId, SortLessStatics>(typeId);
		return index != NO_INDEX ? statics[index].Get() : nullptr;
	}
	bool Context::HasStatic(TypeId typeId) const
	{
		return statics.FindSortedEqual<TypeId, SortLessStatics>(typeId) != NO_INDEX;
	}
	bool Context::RemoveStatic(TypeId typeId)
	{
		const i32 index = statics.FindSortedEqual<TypeId, SortLessStatics>(typeId);
		if (index != NO_INDEX)
		{
			statics.RemoveAt(index);
			return true;
		}
		return false;
	}

	void Context::Reset(bool keepStatics)
	{
		idRegistry = {};
		pools.Clear();
		if (!keepStatics)
		{
			statics.Clear();
		}
	}

	OwnPtr& Context::FindOrAddStaticPtr(TArray<OwnPtr>& statics, const TypeId typeId, bool* bAdded)
	{
		i32 index = statics.LowerBound<TypeId, SortLessStatics>(typeId);
		if (index == NO_INDEX)
		{
			// Insert at the end
			index = statics.Size();
		}
		else if (typeId == statics[index].GetId())
		{
			return statics[index];
		}
		statics.Insert(index, {});
		if (bAdded)
			*bAdded = true;
		return statics[index];
	}
}    // namespace p::ecs
