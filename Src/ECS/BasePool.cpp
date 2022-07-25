// Copyright 2015-2022 Piperift - All rights reserved

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

		idIndices.Reserve(idIndex + 1);
		if (lastRemovedIndex != NO_INDEX && !forceBack)
		{
			const i32 index = lastRemovedIndex;
			idIndices.Insert(idIndex, i32(index));
			lastRemovedIndex = index;
			return index;
		}
		else
		{
			idIndices.Insert(idIndex, idList.Size());
			idList.Add(id);
			return idList.Size() - 1;
		}
	}

	void BasePool::PopId(Id id)
	{
		const Index index = ecs::GetIndex(id);
		i32& listIndex    = idIndices[index];

		idList[listIndex] = ecs::NoId;
		lastRemovedIndex  = listIndex;
		listIndex         = NO_INDEX;
	}

	void BasePool::PopSwapId(Id id)
	{
		const Index index = ecs::GetIndex(id);
		i32& listIndex    = idIndices[i32(index)];

		// Move last element to current index
		idIndices[ecs::GetIndex(idList.Last())] = listIndex;

		idList.RemoveAtSwapUnsafe(listIndex);
		listIndex = NO_INDEX;
	}

	void BasePool::ClearIds()
	{
		if (lastRemovedIndex == NO_INDEX)
		{
			const auto last = end();
			for (auto it = begin(); it < end(); ++it)
			{
				PopId(*it);
			}
		}
		else
		{
			for (Id entity : *this)
			{
				if (ecs::GetVersion(entity) != ecs::GetVersion(ecs::NoId))
				{
					PopId(entity);
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
