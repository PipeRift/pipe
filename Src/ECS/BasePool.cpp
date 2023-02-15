// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/ECS/BasePool.h"

#include "Pipe/Core/Limits.h"


namespace p::ecs
{
	BasePool::BasePool(const BasePool& other)
	    : arena{other.arena}, deletionPolicy{other.deletionPolicy}, context{other.context}
	{
		BindOnPageAllocated();
		idList.Reserve(other.idList.Size());
		idIndices.Reserve(other.idIndices.Capacity());
		for (i32 i = 0; i < other.Size(); ++i)
		{
			const Id id = other.idList[i];
			if (id != ecs::NoId)
			{
				EmplaceId(id, true);
			}
		}
	}

	BasePool::BasePool(BasePool&& other) noexcept
	    : idIndices{Move(other.idIndices)}
	    , idList{Move(other.idList)}
	    , arena{other.arena}
	    , lastRemovedIndex{Exchange(other.lastRemovedIndex, NO_INDEX)}
	    , deletionPolicy{other.deletionPolicy}
	    , context{other.context}
	{}
	BasePool& BasePool::operator=(BasePool&& other) noexcept
	{
		idIndices        = Move(other.idIndices);
		idList           = Move(other.idList);
		arena            = other.arena;
		lastRemovedIndex = Exchange(other.lastRemovedIndex, NO_INDEX);
		deletionPolicy   = other.deletionPolicy;
		context          = other.context;
		return *this;
	}

	BasePool::Index BasePool::EmplaceId(const Id id, bool forceBack)
	{
		CheckMsg(!Has(id), "Set already contains entity");
		const auto idIndex = ecs::GetIndex(id);

		if (lastRemovedIndex != NO_INDEX && !forceBack)
		{
			const i32 index = lastRemovedIndex;
			idIndices.Insert(idIndex, i32(index));
			idList[index]    = id;
			lastRemovedIndex = NO_INDEX;
			return index;
		}
		else
		{
			idIndices.Reserve(idIndex + 1);
			idIndices.Insert(idIndex, idList.Size());
			idList.Add(id);
			return idList.Size() - 1;
		}
	}

	void BasePool::PopId(Id id)
	{
		const Index index = ecs::GetIndex(id);
		i32& idIndex      = idIndices[index];

		idList[idIndex]  = ecs::NoId;
		lastRemovedIndex = idIndex;
		idIndex          = NO_INDEX;
	}

	void BasePool::PopSwapId(Id id)
	{
		i32& idIndex = idIndices[ecs::GetIndex(id)];
		if (idList.RemoveAtSwapUnsafe(idIndex)) [[likely]]
		{
			i32& lastIndex = idIndices[ecs::GetIndex(idList.Last())];
			// Move last element to current index
			idIndex   = lastIndex;
			lastIndex = NO_INDEX;
		}
	}

	void BasePool::ClearIds()
	{
		if (lastRemovedIndex == NO_INDEX)
		{
			const auto last = end();
			for (auto it = begin(); it < last; ++it)
			{
				idIndices[ecs::GetIndex(*it)] = NO_INDEX;
			}
		}
		else
		{
			for (Id entity : *this)
			{
				if (ecs::GetVersion(entity) != ecs::GetVersion(ecs::NoId))
				{
					idIndices[ecs::GetIndex(entity)] = NO_INDEX;
				}
			}
		}

		lastRemovedIndex = NO_INDEX;
		idList.Clear();
	}

	void BasePool::BindOnPageAllocated()
	{
		idIndices.onPageAllocated = [](i32 index, i32* page, i32 size) {
			std::uninitialized_fill_n(page, size, NO_INDEX);
		};
	}

	i32 GetSmallestPool(TSpan<const BasePool*> pools)
	{
		sizet minSize = Limits<sizet>::Max();
		i32 minIndex  = NO_INDEX;
		for (i32 i = 0; i < pools.Size(); ++i)
		{
			const i32 size = pools[i]->Size();
			if (size < minSize)
			{
				minSize  = size;
				minIndex = i;
			}
		}
		return minIndex;
	}
}    // namespace p::ecs
