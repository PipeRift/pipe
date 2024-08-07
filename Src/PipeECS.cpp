// Copyright 2015-2024 Piperift - All rights reserved

#include "PipeECS.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Limits.h"
#include "Pipe/Core/Set.h"


namespace p
{
	Id IdFromString(String str, EntityContext* context)
	{
		if (str == "NoId")
		{
			return NoId;
		}
		const sizet pos = Strings::Find(str, ":");
		if (pos != StringView::npos)
		{
			auto idx = Strings::ToNumber<IdTraits<Id>::Index>({str.data(), str.data() + pos});
			auto v   = Strings::ToNumber<IdTraits<Id>::Version>(
                {str.data() + pos + 1, str.data() + str.size()});
			if (idx && v)
			{
				return MakeId(*idx, *v);
			}
		}
		else if (context)
		{
			if (auto idx = Strings::ToNumber<IdTraits<Id>::Index>(str))
			{
				if (auto v = context->GetIdRegistry().GetValidVersion(*idx))
				{
					return MakeId(*idx, *v);
				}
			}
		}
		return NoId;
	}


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

	void IdRegistry::Create(TView<Id> newIds)
	{
		const i32 availablesUsed = Min(newIds.Size(), available.Size());
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

	bool IdRegistry::Destroy(TView<const Id> ids)
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
		return entities.IsValidIndex(index) && entities[p::i32(index)] == id;
	}

	bool IdRegistry::WasRemoved(Id id) const
	{
		const Index index = GetIdIndex(id);
		return entities.IsValidIndex(index)
		    && GetIdVersion(entities[p::i32(index)]) > GetIdVersion(id);
	}

	TOptional<IdRegistry::Version> IdRegistry::GetValidVersion(IdRegistry::Index idx) const
	{
		if (entities.IsValidIndex(idx))
		{
			return GetIdVersion(entities[p::i32(idx)]);
		}
		return TOptional<Version>{};
	}


	void EntityReader::SerializeEntities(
	    TArray<Id>& entities, TFunction<void(EntityReader&)> onReadPools)
	{
		TArray<Id> parents;
		GetIdParent(context, entities, parents);

		i32 idCount = 0;
		Next("count", idCount);
		ids.Resize(i32(idCount));
		// Create or assign root ids
		const i32 maxSize = Min(entities.Size(), ids.Size());
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

		FixParentIdLinks(context, parents);
	}

	void EntityReader::SerializeEntity(Id& entity, TFunction<void(EntityReader&)> onReadPools)
	{
		TArray<Id> entities{entity};
		SerializeEntities(entities, onReadPools);
		entity = entities.IsEmpty() ? NoId : entities[0];
	}

	void EntityReader::SerializeSingleEntity(Id& entity, TFunction<void(EntityReader&)> onReadPools)
	{
		Id parent = GetIdParent(context, entity);
		if (entity == NoId)
		{
			entity = context.Create();
		}
		ids.Assign(&entity, 1);

		onReadPools(*this);

		FixParentIdLinks(context, parent);
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
			GetIdChildren(context, pendingInspection, currentLinked);
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
		auto* entityReader = p::Cast<EntityReader*>(&r);
		if (P_EnsureMsg(entityReader, "Serializing an ecs Id without an EntityReader")) [[likely]]
		{
			i32 dataId;
			entityReader->Serialize(dataId);
			val = dataId >= 0 ? entityReader->GetIds()[dataId] : NoId;
		}
	}

	void Write(Writer& w, Id val)
	{
		auto* entityWriter = p::Cast<EntityWriter*>(&w);
		if (P_EnsureMsg(entityWriter, "Serializing an ecs Id without an EntityWriter")) [[likely]]
		{
			const i32* dataId = entityWriter->GetIdToIndexes().Find(val);
			entityWriter->Serialize(dataId ? *dataId : -1);
		}
	}


	BasePool::BasePool(const BasePool& other)
	    : idIndices{*other.arena}
	    , idList{*other.arena}
	    , arena{other.arena}
	    , deletionPolicy{other.deletionPolicy}
	    , typeId{other.typeId}
	    , context{other.context}
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
	    , typeId{Exchange(other.typeId, {})}
	    , context{other.context}
	{}
	BasePool& BasePool::operator=(BasePool&& other) noexcept
	{
		idIndices        = Move(other.idIndices);
		idList           = Move(other.idList);
		arena            = other.arena;
		lastRemovedIndex = Exchange(other.lastRemovedIndex, NO_INDEX);
		deletionPolicy   = other.deletionPolicy;
		typeId           = other.typeId;
		context          = other.context;
		return *this;
	}

	BasePool::Index BasePool::EmplaceId(const Id id, bool forceBack)
	{
		P_CheckMsg(!Has(id), "Set already contains entity");
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
		idList.RemoveAtSwapUnsafe(idIndex);

		i32& lastIndex = idIndices[GetIdIndex(idList.Last())];
		// Move last element to current index
		idIndex   = lastIndex;
		lastIndex = NO_INDEX;
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

	i32 GetSmallestPool(TView<const BasePool* const> pools)
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
	void EntityContext::Create(TView<Id> ids)
	{
		idRegistry.Create(ids);
	}

	void EntityContext::Destroy(const Id id)
	{
		for (auto& pool : pools)
		{
			pool.GetPool()->Remove(id);
		}
		idRegistry.Destroy(id);
	}

	void EntityContext::Destroy(TView<const Id> ids)
	{
		for (auto& pool : pools)
		{
			pool.GetPool()->Remove(ids);
		}
		idRegistry.Destroy(ids);
	}

	void* EntityContext::AddDefault(TypeId typeId, Id id)
	{
		if (BasePool* pool = GetPool(typeId))
		{
			return pool->AddDefault(id);
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
		const i32 index = pools.FindSorted(PoolInstance{componentId, {}});
		return index != NO_INDEX ? pools[index].GetPool() : nullptr;
	}

	void EntityContext::GetPools(
	    TView<const TypeId> componentIds, TArray<BasePool*>& outPools) const
	{
		for (const TypeId componentId : componentIds)
		{
			const i32 index = pools.FindSorted(PoolInstance{componentId, {}});
			if (index != NO_INDEX)
			{
				outPools.Add(pools[index].GetPool());
			}
		}
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

	bool EntityContext::WasRemoved(Id id) const
	{
		return idRegistry.WasRemoved(id);
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
		const i32 index = statics.FindSorted<TypeId, SortLessStatics>(typeId);
		return index != NO_INDEX ? statics[index].Get() : nullptr;
	}
	const void* EntityContext::TryGetStatic(TypeId typeId) const
	{
		const i32 index = statics.FindSorted<TypeId, SortLessStatics>(typeId);
		return index != NO_INDEX ? statics[index].Get() : nullptr;
	}
	bool EntityContext::HasStatic(TypeId typeId) const
	{
		return statics.FindSorted<TypeId, SortLessStatics>(typeId) != NO_INDEX;
	}
	bool EntityContext::RemoveStatic(TypeId typeId)
	{
		const i32 index = statics.FindSorted<TypeId, SortLessStatics>(typeId);
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
		statics.Insert(index);
		if (bAdded)
			*bAdded = true;
		return statics[index];
	}


	void ExcludeIdsWith(const BasePool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
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

	void ExcludeIdsWithStable(const BasePool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ids.RemoveIf(
		    [pool](Id id) {
			return pool->Has(id);
		    },
		    shouldShrink);
	}

	void ExcludeIdsWithout(const BasePool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
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

	void ExcludeIdsWithoutStable(const BasePool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ids.RemoveIf(
		    [pool](Id id) {
			return !pool->Has(id);
		    },
		    shouldShrink);
	}

	void FindIdsWith(const BasePool* pool, TView<const Id> source, TArray<Id>& results)
	{
		if (pool)
		{
			results.ReserveMore(Min(i32(pool->Size()), source.Size()));
			for (Id id : source)
			{
				if (pool->Has(id))
				{
					results.Add(id);
				}
			}
		}
	}
	void FindIdsWith(
	    TView<const BasePool* const> pools, TView<const Id> source, TArray<Id>& results)
	{
		FindIdsWith(pools.First(), source, results);
		for (i32 i = 1; i < pools.Size(); ++i)
		{
			ExcludeIdsWithout(pools[i], results, false);
		}
	}

	void FindIdsWithout(const BasePool* pool, TView<const Id> source, TArray<Id>& results)
	{
		if (pool)
		{
			results.ReserveMore(source.Size());
			for (Id id : source)
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

	void ExtractIdsWith(
	    const BasePool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		results.ReserveMore(Min(i32(pool->Size()), source.Size()));
		for (i32 i = source.Size() - 1; i >= 0; --i)
		{
			const Id id = source[i];
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

	void ExtractIdsWithStable(
	    const BasePool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		results.ReserveMore(Min(i32(pool->Size()), source.Size()));
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

	void ExtractIdsWithout(
	    const BasePool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		results.ReserveMore(source.Size());
		for (i32 i = source.Size() - 1; i >= 0; --i)
		{
			const Id id = source[i];
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

	void ExtractIdsWithoutStable(
	    const BasePool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
	{
		results.ReserveMore(Min(i32(pool->Size()), source.Size()));
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

	void FindAllIdsWith(TView<const BasePool* const> pools, TArray<Id>& ids)
	{
		if (pools.IsEmpty())
			return;

		for (const BasePool* pool : pools)
		{
			if (!P_EnsureMsg(pool,
			        "One of the pools is null. Is the access missing one or more of the specified "
			        "components?"))
			{
				return;
			}
		}

		const i32 smallestIdx        = GetSmallestPool(pools);
		const BasePool* iterablePool = pools[smallestIdx];

		ids.Clear(false);
		ids.Reserve(iterablePool->Size());
		for (Id id : *iterablePool)
		{
			if (!IsNone(id)) [[likely]]
			{
				ids.Add(id);
			}
		}
		// Faster but doesn't exclude invalid ids.
		// ids.Append(iterablePool->begin(), iterablePool->end());

		for (i32 i = 0; i < pools.Size(); ++i)
		{
			if (i != smallestIdx)
			{
				const BasePool* pool = pools[i];
				ExcludeIdsWithout(pool, ids, false);
			}
		}
	}

	void FindAllIdsWithAny(TView<const BasePool* const> pools, TArray<Id>& ids)
	{
		if (pools.IsEmpty())
			return;

		for (const BasePool* pool : pools)
		{
			if (!P_EnsureMsg(pool,
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

	void FindAllIdsWithAnyUnique(TView<const BasePool* const> pools, TArray<Id>& ids)
	{
		if (pools.IsEmpty())
			return;

		i32 maxPossibleSize = 0;
		for (const BasePool* pool : pools)
		{
			if (!P_EnsureMsg(pool,
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

	void Read(Reader& ct, CParent& val)
	{
		ct.Serialize(val.children);
	}
	void Write(Writer& ct, const CParent& val)
	{
		ct.Serialize(val.children);
	}

	void Read(Reader& ct, CChild& val)
	{
		ct.Serialize(val.parent);
	}
	void Write(Writer& ct, const CChild& val)
	{
		ct.Serialize(val.parent);
	}


	void RemoveChildFromCParent(TAccessRef<TWrite<CParent>> access, Id parent, Id child)
	{
		if (auto* cParent = access.TryGet<CParent>(parent))
		{
			cParent->children.Remove(child);
			if (cParent->children.IsEmpty())
			{
				access.Remove<CParent>(parent);
			}
		}
	}

	void AttachId(
	    TAccessRef<TWrite<CChild>, TWrite<CParent>> access, Id parent, TView<const Id> children)
	{
		children.Each([&access, parent](Id childId) {
			if (auto* cChild = access.TryGet<CChild>(childId))
			{
				if (cChild->parent == parent
				    || !P_EnsureMsg(IsNone(cChild->parent),
				        "An entity trying to be attached already has a parent. Consider using "
				        "TransferIdChildren()"))
				{
					return;
				}
				cChild->parent = parent;
			}
			else
			{
				access.Add<CChild>(childId).parent = parent;
			}
		});
		access.GetOrAdd<CParent>(parent).children.Append(children);
	}

	void AttachIdAfter(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, Id parent,
	    TView<Id> childrenIds, Id prevChild)
	{
		childrenIds.Each([&access, parent](Id child) {
			if (auto* cChild = access.TryGet<CChild>(child))
			{
				if (P_EnsureMsg(IsNone(cChild->parent),
				        "An entity trying to be attached already has a parent. Consider using "
				        "TransferIdChildren()"))
				{
					cChild->parent = parent;
				}
			}
			else
			{
				access.Add<CChild>(child).parent = parent;
			}
		});

		auto& childrenList  = access.GetOrAdd<CParent>(parent).children;
		const i32 prevIndex = childrenList.FindIndex(prevChild);
		childrenList.Insert(prevIndex, childrenIds);
	}

	void TransferIdChildren(TAccessRef<TWrite<CChild>, TWrite<CParent>> access,
	    TView<const Id> childrenIds, Id destination)
	{
		DetachIdParent(access, childrenIds, true);
		AttachId(access, destination, childrenIds);
	}

	void DetachIdParent(TAccessRef<TWrite<CParent>, TWrite<CChild>> access,
	    TView<const Id> childrenIds, bool keepComponents)
	{
		TArray<Id> parents;
		parents.Reserve(childrenIds.Size());

		childrenIds.Each([&access, &parents](Id child) {
			if (auto* cChild = access.TryGet<CChild>(child))
			{
				parents.Add(cChild->parent);
				cChild->parent = NoId;
			}
		});

		if (!keepComponents)
		{
			childrenIds.Each([&access](Id child) {
				access.Remove<CChild>(child);
			});
		}

		// Sort to iterate avoiding duplicated parents
		parents.Sort();
		Id lastParent = NoId;

		if (keepComponents)
		{
			for (Id parent : parents)
			{
				if (parent == lastParent)
				{
					continue;
				}
				lastParent = parent;

				if (auto* cParent = access.TryGet<CParent>(parent))
				{
					cParent->children.Remove(childrenIds);
				}
			}
		}
		else
		{
			for (Id parent : parents)
			{
				if (parent == lastParent)
				{
					continue;
				}
				lastParent = parent;

				if (auto* cParent = access.TryGet<CParent>(parent))
				{
					cParent->children.Remove(childrenIds);
					if (cParent->children.IsEmpty())
					{
						access.Remove<CParent>(parent);
					}
				}
			}
		}
	}

	void DetachIdChildren(TAccessRef<TWrite<CParent>, TWrite<CChild>> access,
	    TView<const Id> parents, bool keepComponents)
	{
		if (keepComponents)
		{
			parents.Each([&access](Id parent) {
				if (auto* cParent = access.TryGet<CParent>(parent))
				{
					for (Id child : cParent->children)
					{
						access.Get<CChild>(parent).parent = NoId;
					}
					cParent->children.Clear();
				}
			});
		}
		else
		{
			parents.Each([&access](Id parent) {
				if (auto* cParent = access.TryGet<CParent>(parent))
				{
					for (Id child : cParent->children)
					{
						access.Remove<CChild>(child);
					}
					access.Remove<CParent>(parent);
				}
			});
		}
	}

	const TArray<Id>* GetIdChildren(TAccessRef<CParent> access, Id node)
	{
		auto* cParent = access.TryGet<const CParent>(node);
		return cParent ? &cParent->children : nullptr;
	}

	void GetIdChildren(
	    TAccessRef<CParent> access, TView<const Id> parentIds, TArray<Id>& outChildrenIds)
	{
		parentIds.Each([&access, &outChildrenIds](Id id) {
			if (const auto* cParent = access.TryGet<const CParent>(id))
			{
				outChildrenIds.Append(cParent->children);
			}
		});
	}

	void GetAllIdChildren(TAccessRef<CParent> access, TView<const Id> parentIds,
	    TArray<Id>& outChildrenIds, u32 depth)
	{
		P_Check(depth > 0);

		TArray<Id> currentLinked{};
		TArray<Id> pendingInspection;
		pendingInspection.Append(parentIds);
		while (pendingInspection.Size() > 0 && --depth >= 0)
		{
			GetIdChildren(access, pendingInspection, currentLinked);
			outChildrenIds.Append(currentLinked);
			pendingInspection = Move(currentLinked);
		}
	}

	Id GetIdParent(TAccessRef<CChild> access, Id childId)
	{
		if (const auto* cChild = access.TryGet<const CChild>(childId))
		{
			return cChild->parent;
		}
		return NoId;
	}

	void GetIdParent(TAccessRef<CChild> access, TView<const Id> childrenIds, TArray<Id>& outParents)
	{
		outParents.Clear(false);
		for (Id childId : childrenIds)
		{
			const auto* child = access.TryGet<const CChild>(childId);
			if (child && !IsNone(child->parent))
			{
				outParents.AddUnique(child->parent);
			}
		}
	}
	void GetAllIdParents(
	    TAccessRef<CChild> access, TView<const Id> childrenIds, TArray<Id>& outParents)
	{
		outParents.Clear(false);

		TArray<Id> currentIds{childrenIds.begin(), childrenIds.end()};
		TArray<Id> parentIds;

		while (currentIds.Size() > 0)
		{
			GetIdParent(access, currentIds, parentIds);
			outParents.Append(parentIds);
			Swap(currentIds, parentIds);
			parentIds.Clear(false);
		}
	}

	Id FindIdParent(TAccessRef<CChild> access, Id childId, const TFunction<bool(Id)>& callback)
	{
		while (!IsNone(childId))
		{
			childId = GetIdParent(access, childId);
			if (callback(childId))
			{
				return childId;
			}
		}
		return NoId;
	}
	void FindIdParents(TAccessRef<CChild> access, TView<const Id> childrenIds,
	    TArray<Id>& outParentIds, const TFunction<bool(Id)>& callback)
	{
		outParentIds.Clear(false);

		TArray<Id> currentIds{childrenIds};
		TArray<Id> parentIds;

		while (currentIds.Size() > 0)
		{
			GetIdParent(access, currentIds, parentIds);
			for (i32 i = 0; i < parentIds.Size();)
			{
				const Id parentId = parentIds[i];
				if (callback(parentId))
				{
					outParentIds.Add(parentId);
					parentIds.RemoveAtSwap(i, false);
				}
				else
				{
					++i;
				}
			}
			Swap(currentIds, parentIds);
			parentIds.Clear(false);
		}
	}

	void RemoveId(TAccessRef<TWrite<CChild>, TWrite<CParent>> access, TView<Id> ids, bool deep)
	{
		DetachIdParent(access, ids, true);

		if (deep)
		{
			TArray<Id> allIds;
			allIds.Append(ids);
			GetAllIdChildren(access, ids, allIds);
			// No children to detach since we will remove all of them
			access.GetContext().Destroy(allIds);
		}
		else
		{
			DetachIdChildren(access, ids);
			access.GetContext().Destroy(ids);
		}
	}


	bool FixParentIdLinks(TAccessRef<TWrite<CChild>, CParent> access, TView<Id> parents)
	{
		bool fixed = false;
		for (Id parentId : parents)
		{
			if (const auto* parent = access.TryGet<const CParent>(parentId))
			{
				for (Id childId : parent->children)
				{
					if (auto* child = access.TryGet<CChild>(childId))
					{
						if (child->parent != parentId)
						{
							child->parent = parentId;
							fixed         = true;
						}
					}
					else
					{
						access.Add<CChild>(childId, {parentId});
						fixed = true;
					}
				}
			}
		}
		return fixed;
	}

	bool ValidateParentIdLinks(TAccessRef<CChild, CParent> access, TView<Id> parents)
	{
		for (Id parentId : parents)
		{
			if (const auto* parent = access.TryGet<const CParent>(parentId))
			{
				for (Id childId : parent->children)
				{
					const auto* child = access.TryGet<const CChild>(childId);
					if (!child || child->parent != parentId)
					{
						return false;
					}
				}
			}
		}
		return true;
	}

	void GetRootIds(TAccessRef<CChild, CParent> access, TArray<Id>& outRoots)
	{
		outRoots = FindAllIdsWith<CParent>(access);
		ExcludeIdsWith<CChild>(access, outRoots);
	}
}    // namespace p
