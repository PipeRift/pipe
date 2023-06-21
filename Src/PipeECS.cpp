// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/PipeECS.h"

#include "Pipe/Core/Limits.h"
#include "Pipe/ECS/Utils/Hierarchy.h"


namespace p
{
	Id IdRegistry::Create()
	{
		if (!available.IsEmpty())
		{
			const Index index = available.Last();
			available.RemoveLast();
			return entities[index];
		}

		const Id id = MakeId(entities.Size(), 0);
		entities.Add(id);
		return id;
	}

	void IdRegistry::Create(TSpan<Id> newIds)
	{
		const i32 availablesUsed = math::Min(newIds.Size(), available.Size());
		for (i32 i = 0; i < availablesUsed; ++i)
		{
			const Index index = available.Last();
			newIds[i]         = entities[index];
		}
		available.RemoveLast(availablesUsed);

		// Remaining entities
		const u32 remainingSize = newIds.Size() - availablesUsed;
		entities.Reserve(entities.Size() + remainingSize);
		for (i32 i = availablesUsed; i < newIds.Size(); ++i)
		{
			const Id id = MakeId(entities.Size(), 0);
			entities.Add(id);
			newIds[i] = id;
		}
	}

	bool IdRegistry::Destroy(Id id)
	{
		const Index index = GetIdIndex(id);
		if (entities.IsValidIndex(index))
		{
			Id& storedId = entities[index];
			if (id == storedId)
			{
				// Increase version to invalidate current entity
				storedId = MakeId(index, GetIdVersion(storedId) + 1u);
				available.Add(index);
				return true;
			}
		}
		return false;
	}

	bool IdRegistry::Destroy(TSpan<const Id> ids)
	{
		available.Reserve(available.Size() + ids.Size());
		const u32 lastAvailable = available.Size();
		for (Id id : ids)
		{
			const Index index = GetIdIndex(id);
			if (entities.IsValidIndex(index))
			{
				Id& storedId = entities[index];
				if (id == storedId)
				{
					// Increase version to invalidate current entity
					storedId = MakeId(index, GetIdVersion(storedId) + 1u);
					available.Add(index);
				}
			}
		}
		return (available.Size() - lastAvailable) > 0;
	}

	bool IdRegistry::IsValid(Id id) const
	{
		const Index index = GetIdIndex(id);
		return entities.IsValidIndex(index) && entities[index] == id;
	}


	void EntityReader::SerializeEntities(
	    TArray<Id>& entities, TFunction<void(EntityReader&)> onReadPools)
	{
		TArray<Id> parents;
		ecs::GetParents(context, entities, parents);

		i32 idCount = 0;
		Next("count", idCount);
		ids.Resize(i32(idCount));
		// Create or assign root ids
		const i32 maxSize = math::Min(entities.Size(), ids.Size());
		for (i32 i = 0; i < maxSize; ++i)
		{
			Id& entity = entities[i];
			if (entity == NoId)
			{
				entity = context.Create();
			}
			ids[i] = entity;
		}
		// Create all non-root entities
		context.Create({ids.Data() + maxSize, ids.Data() + ids.Size()});

		if (EnterNext("components"))
		{
			BeginObject();
			serializingMany = true;
			onReadPools(*this);
			serializingMany = false;
			Leave();
		}

		ecs::FixParentLinks(context, parents);
	}

	void EntityReader::SerializeEntity(Id& entity, TFunction<void(EntityReader&)> onReadPools)
	{
		TArray<Id> entities{entity};
		SerializeEntities(entities, onReadPools);
		entity = entities.IsEmpty() ? NoId : entities[0];
	}

	void EntityReader::SerializeSingleEntity(Id& entity, TFunction<void(EntityReader&)> onReadPools)
	{
		Id parent = ecs::GetParent(context, entity);
		if (entity == NoId)
		{
			entity = context.Create();
		}
		ids.Assign(&entity, 1);

		onReadPools(*this);

		ecs::FixParentLinks(context, parent);
	}

	const TArray<Id>& EntityReader::GetIds() const
	{
		return ids;
	}

	EntityContext& EntityReader::GetContext()
	{
		return context;
	}


	void EntityWriter::SerializeEntities(const TArray<Id>& entities,
	    TFunction<void(EntityWriter&)> onWritePools, bool includeChildren)
	{
		if (includeChildren)
			RetrieveHierarchy(entities, ids);
		else
			ids = entities;
		MapIdsToIndices();

		Next("count", ids.Size());
		if (EnterNext("components"))
		{
			BeginObject();
			serializingMany = true;
			onWritePools(*this);
			serializingMany = false;
			Leave();
		}
	}

	void EntityWriter::SerializeEntity(
	    Id entity, TFunction<void(EntityWriter&)> onWritePools, bool includeChildren)
	{
		SerializeEntities({entity}, onWritePools, includeChildren);
	}
	void EntityWriter::SerializeSingleEntity(Id entity, TFunction<void(EntityWriter&)> onWritePools)
	{
		ids.Assign(&entity, 1);
		MapIdsToIndices();

		onWritePools(*this);
	}

	const TArray<Id>& EntityWriter::GetIds() const
	{
		return ids;
	}

	const TMap<Id, i32>& EntityWriter::GetIdToIndexes() const
	{
		return idToIndexes;
	}

	void EntityWriter::RetrieveHierarchy(const TArray<Id>& roots, TArray<Id>& children)
	{
		children.Append(roots);

		TArray<Id> currentLinked{};
		TArray<Id> pendingInspection;
		pendingInspection.Append(roots);
		while (pendingInspection.Size() > 0)
		{
			RemoveIgnoredEntities(pendingInspection);
			ecs::GetChildren(context, pendingInspection, currentLinked);
			children.Append(currentLinked);
			pendingInspection = Move(currentLinked);
		}
	}

	void EntityWriter::RemoveIgnoredEntities(TArray<Id>& entities)
	{
		TAccess<CNotSerialized> access{context};
		for (i32 i = 0; i < entities.Size(); ++i)
		{
			if (access.Has<CNotSerialized>(entities[i]))
			{
				entities.RemoveAtSwapUnsafe(i);
				--i;
			}
		}
		entities.Shrink();
	}

	void EntityWriter::MapIdsToIndices()
	{
		idToIndexes.Reserve(ids.Size());
		for (i32 i = 0; i < ids.Size(); ++i)
		{
			idToIndexes.Insert(ids[i], i);
		}
	}

	void Read(Reader& r, Id& val)
	{
		auto* entityReader = dynamic_cast<EntityReader*>(&r);
		if (EnsureMsg(entityReader, "Serializing an ecs Id without an EntityReader")) [[likely]]
		{
			i32 dataId;
			entityReader->Serialize(dataId);
			val = dataId >= 0 ? entityReader->GetIds()[dataId] : NoId;
		}
	}

	void Write(Writer& w, Id val)
	{
		auto* entityWriter = dynamic_cast<EntityWriter*>(&w);
		if (EnsureMsg(entityWriter, "Serializing an ecs Id without an EntityWriter")) [[likely]]
		{
			const i32* dataId = entityWriter->GetIdToIndexes().Find(val);
			entityWriter->Serialize(dataId ? *dataId : -1);
		}
	}


	BasePool::BasePool(const BasePool& other)
	    : arena{other.arena}, deletionPolicy{other.deletionPolicy}, context{other.context}
	{
		BindOnPageAllocated();
		idList.Reserve(other.idList.Size());
		idIndices.Reserve(other.idIndices.Capacity());
		for (i32 i = 0; i < other.Size(); ++i)
		{
			const Id id = other.idList[i];
			if (GetIdVersion(id) != NoIdVersion)
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
		const auto idIndex = GetIdIndex(id);

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
		const Index index = GetIdIndex(id);
		i32& idIndex      = idIndices[index];

		idList[idIndex]  = MakeId(index, NoIdVersion);    // Mark invalid but keep index
		lastRemovedIndex = idIndex;
		idIndex          = NO_INDEX;
	}

	void BasePool::PopSwapId(Id id)
	{
		i32& idIndex = idIndices[GetIdIndex(id)];
		if (idList.RemoveAtSwapUnsafe(idIndex)) [[likely]]
		{
			i32& lastIndex = idIndices[GetIdIndex(idList.Last())];
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
			for (Id id : idList)
			{
				idIndices[GetIdIndex(id)] = NO_INDEX;
			}
		}
		else
		{
			for (Id id : idList)
			{
				if (GetIdVersion(id) != NoIdVersion)
				{
					idIndices[GetIdIndex(id)] = NO_INDEX;
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


	PoolInstance::PoolInstance(TypeId componentId, TUniquePtr<BasePool>&& pool)
	    : componentId{componentId}, pool{Move(pool)}
	{}
	PoolInstance::PoolInstance(PoolInstance&& other) noexcept
	{
		componentId       = other.componentId;
		other.componentId = TypeId::None();
		pool              = Move(other.pool);
	}
	PoolInstance::PoolInstance(const PoolInstance& other)
	{
		componentId = other.componentId;
		if (other.pool)
		{
			pool = other.pool->Clone();
		}
	}
	PoolInstance& PoolInstance::operator=(PoolInstance&& other) noexcept
	{
		componentId       = other.componentId;
		other.componentId = TypeId::None();
		pool              = Move(other.pool);
		return *this;
	}
	PoolInstance& PoolInstance::operator=(const PoolInstance& other)
	{
		componentId = other.componentId;
		pool.Reset();
		if (other.pool)
		{
			pool = other.pool->Clone();
		}
		return *this;
	}

	TypeId PoolInstance::GetId() const
	{
		return componentId;
	}

	BasePool* PoolInstance::GetPool() const
	{
		return pool.Get();
	}

	bool PoolInstance::operator<(const PoolInstance& other) const
	{
		return componentId.GetId() < other.componentId.GetId();
	}


	EntityContext::EntityContext() {}

	EntityContext::EntityContext(const EntityContext& other) noexcept
	{
		CopyFrom(other);
	}
	EntityContext::EntityContext(EntityContext&& other) noexcept
	{
		MoveFrom(Move(other));
	}
	EntityContext& EntityContext::operator=(const EntityContext& other) noexcept
	{
		Reset();
		CopyFrom(other);
		return *this;
	}
	EntityContext& EntityContext::operator=(EntityContext&& other) noexcept
	{
		Reset();
		MoveFrom(Move(other));
		return *this;
	}

	Id EntityContext::Create()
	{
		return idRegistry.Create();
	}
	void EntityContext::Create(TSpan<Id> ids)
	{
		idRegistry.Create(ids);
	}

	void EntityContext::Destroy(const Id id)
	{
		idRegistry.Destroy(id);
		for (auto& pool : pools)
		{
			pool.GetPool()->Remove(id);
		}
	}

	void EntityContext::Destroy(TSpan<const Id> ids)
	{
		idRegistry.Destroy(ids);
		for (auto& pool : pools)
		{
			pool.GetPool()->Remove(ids);
		}
	}

	void* EntityContext::AddDefaulted(TypeId typeId, Id id)
	{
		if (BasePool* pool = GetPool(typeId))
		{
			return pool->AddDefaulted(id);
		}
		return nullptr;
	}

	void EntityContext::Remove(TypeId typeId, Id id)
	{
		if (BasePool* pool = GetPool(typeId))
		{
			pool->Remove(id);
		}
	}

	BasePool* EntityContext::GetPool(TypeId componentId) const
	{
		const i32 index = pools.FindSortedEqual(PoolInstance{componentId, {}});
		return index != NO_INDEX ? pools[index].GetPool() : nullptr;
	}

	void EntityContext::CopyFrom(const EntityContext& other)
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

	void EntityContext::MoveFrom(EntityContext&& other)
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

	bool EntityContext::IsValid(Id id) const
	{
		return idRegistry.IsValid(id);
	}

	bool EntityContext::IsOrphan(const Id id) const
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

	void* EntityContext::TryGetStatic(TypeId typeId)
	{
		const i32 index = statics.FindSortedEqual<TypeId, SortLessStatics>(typeId);
		return index != NO_INDEX ? statics[index].Get() : nullptr;
	}
	const void* EntityContext::TryGetStatic(TypeId typeId) const
	{
		const i32 index = statics.FindSortedEqual<TypeId, SortLessStatics>(typeId);
		return index != NO_INDEX ? statics[index].Get() : nullptr;
	}
	bool EntityContext::HasStatic(TypeId typeId) const
	{
		return statics.FindSortedEqual<TypeId, SortLessStatics>(typeId) != NO_INDEX;
	}
	bool EntityContext::RemoveStatic(TypeId typeId)
	{
		const i32 index = statics.FindSortedEqual<TypeId, SortLessStatics>(typeId);
		if (index != NO_INDEX)
		{
			statics.RemoveAt(index);
			return true;
		}
		return false;
	}

	void EntityContext::Reset(bool keepStatics)
	{
		idRegistry = {};
		pools.Clear();
		if (!keepStatics)
		{
			statics.Clear();
		}
	}

	OwnPtr& EntityContext::FindOrAddStaticPtr(
	    TArray<OwnPtr>& statics, const TypeId typeId, bool* bAdded)
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
}    // namespace p
