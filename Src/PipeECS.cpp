// Copyright 2015-2026 Piperift. All Rights Reserved.

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
			auto idx = Strings::ToNumber<Id::Index>({str.data(), str.data() + pos});
			auto v =
			    Strings::ToNumber<Id::Version>({str.data() + pos + 1, str.data() + str.size()});
			if (idx && v)
			{
				return MakeId(*idx, *v);
			}
		}
		else if (context)
		{
			if (auto idx = Strings::ToNumber<Id::Index>(str))
			{
				if (auto v = context->GetIdRegistry().GetValidVersion(*idx))
				{
					return MakeId(*idx, *v);
				}
			}
		}
		return NoId;
	}


	IdRegistry::IdRegistry(IdRegistry&& other)
	{
		std::unique_lock lock{other.mutex};
		entities        = Move(other.entities);
		available       = Move(other.available);
		deferredRemoves = Move(other.deferredRemoves);
	}
	IdRegistry::IdRegistry(const IdRegistry& other)
	{
		std::shared_lock lock{other.mutex};
		entities        = other.entities;
		available       = other.available;
		deferredRemoves = other.deferredRemoves;
	}
	IdRegistry& IdRegistry::operator=(IdRegistry&& other)
	{
		std::unique_lock lock{other.mutex};
		entities        = Move(other.entities);
		available       = Move(other.available);
		deferredRemoves = Move(other.deferredRemoves);
		return *this;
	}
	IdRegistry& IdRegistry::operator=(const IdRegistry& other)
	{
		std::shared_lock lock{other.mutex};
		entities        = other.entities;
		available       = other.available;
		deferredRemoves = other.deferredRemoves;
		return *this;
	}

	Id IdRegistry::Create()
	{
		std::unique_lock lock{mutex};
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
		std::unique_lock lock{mutex};
		const i32 nRecicled = Min(newIds.Size(), available.Size());
		if (nRecicled > 0)
		{
			for (i32 i = 0; i < nRecicled; ++i)
			{
				const Index index = available.Last();
				newIds[i]         = entities[index];
			}
			available.RemoveLast(nRecicled);
			newIds = newIds.LastUnsafe(newIds.Size() - nRecicled);
		}

		// Remaining entities
		const i32 firstIndex = entities.Size();
		for (i32 i = 0; i < newIds.Size(); ++i)
		{
			newIds[i] = MakeId(firstIndex + i, 0);
		}
		entities.Append(newIds);
	}

	bool IdRegistry::Remove(TView<const Id> ids)
	{
		std::unique_lock lock{mutex};
		available.ReserveMore(ids.Size());
		const u32 lastAvailable = available.Size();
		for (Id id : ids)
		{
			const Index index = id.GetIndex();
			if (entities.IsValidIndex(index))
			{
				Id& storedId = entities[index];
				if (id == storedId)
				{
					// Increase version to invalidate current entity
					storedId = MakeId(index, storedId.GetVersion() + 1u);
					available.Add(index);
				}
			}
		}
		return (available.Size() - lastAvailable) > 0;
	}

	bool IdRegistry::DeferredRemove(TView<const Id> ids)
	{
		std::unique_lock lock{mutex};
		deferredRemoves.ReserveMore(ids.Size());
		const u32 lastPending = deferredRemoves.Size();
		for (Id id : ids)
		{
			const Index index = id.GetIndex();
			if (entities.IsValidIndex(index))
			{
				Id& storedId = entities[index];
				if (id == storedId)
				{
					deferredRemoves.Add(storedId);
					// Increase version to invalidate current entity
					storedId = MakeId(index, storedId.GetVersion() + 1u);
				}
			}
		}
		return (deferredRemoves.Size() - lastPending) > 0;
	}

	bool IdRegistry::FlushDeferredRemovals()
	{
		if (deferredRemoves.Size() > 0)
		{
			available.ReserveMore(deferredRemoves.Size());
			for (Id id : deferredRemoves)
			{
				available.Add(id.GetIndex());
			}
			deferredRemoves.Clear();
			return true;
		}
		return false;
	}

	bool IdRegistry::IsValid(Id id) const
	{
		std::shared_lock lock{mutex};
		const Index index = id.GetIndex();
		return entities.IsValidIndex(index) && entities[p::i32(index)] == id;
	}

	bool IdRegistry::WasRemoved(Id id) const
	{
		std::shared_lock lock{mutex};
		const Index index = id.GetIndex();
		return entities.IsValidIndex(index)
		    && entities[p::i32(index)].GetVersion() > id.GetVersion();
	}

	TOptional<IdRegistry::Version> IdRegistry::GetValidVersion(IdRegistry::Index idx) const
	{
		std::shared_lock lock{mutex};
		if (entities.IsValidIndex(idx))
		{
			return entities[p::i32(idx)].GetVersion();
		}
		return TOptional<Version>{};
	}

	u32 IdRegistry::Size() const
	{
		return entities.Size() - available.Size() - deferredRemoves.Size();
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
				entity = AddId(context);
			}
			ids[i] = entity;
		}
		// Create all non-root entities
		AddId(context, {ids.Data() + maxSize, ids.Data() + ids.Size()});

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
			entity = AddId(context);
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
		{
			RetrieveHierarchy(entities, ids);
		}
		else
		{
			ids = entities;
		}
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
		auto* entityReader = p::Cast<EntityReader>(&r);
		if (P_EnsureMsg(entityReader, "Serializing an ecs Id without an EntityReader")) [[likely]]
		{
			i32 dataId;
			entityReader->Serialize(dataId);
			val = dataId >= 0 ? entityReader->GetIds()[dataId] : NoId;
		}
	}

	void Write(Writer& w, Id val)
	{
		auto* entityWriter = p::Cast<EntityWriter>(&w);
		if (P_EnsureMsg(entityWriter, "Serializing an ecs Id without an EntityWriter")) [[likely]]
		{
			const i32* dataId = entityWriter->GetIdToIndexes().Find(val);
			entityWriter->Serialize(dataId ? *dataId : -1);
		}
	}


	IPool::Iterator IPool::begin() const
	{
		const TArray<Id>& idList = GetIdList();
		return Iterator{idList, static_cast<Index>(idList.Size())};
	}
	IPool::Iterator IPool::end() const
	{
		return Iterator{GetIdList(), {}};
	}
	IPool::Iterator IPool::cbegin() const
	{
		return begin();
	}
	IPool::Iterator IPool::cend() const
	{
		return end();
	}
	IPool::ReverseIterator IPool::rbegin() const
	{
		return std::make_reverse_iterator(end());
	}
	IPool::ReverseIterator IPool::rend() const
	{
		return std::make_reverse_iterator(begin());
	}
	IPool::ReverseIterator IPool::crbegin() const
	{
		return rbegin();
	}
	IPool::ReverseIterator IPool::crend() const
	{
		return rend();
	}


	ComponentPool::ComponentPool(TypeId typeId, PoolRemovePolicy removePolicy, Arena& arena)
	    : IPool(typeId), idIndices{arena}, idList{arena}, arena{&arena}, removePolicy{removePolicy}
	{
		BindOnPageAllocated();
	}

	ComponentPool::ComponentPool(const ComponentPool& other)
	    : IPool(other.typeId), idIndices{*other.arena}, idList{*other.arena}
	{
		arena        = other.arena;
		removePolicy = other.removePolicy;
		typeId       = other.typeId;
		BindOnPageAllocated();
		idList.Reserve(other.idList.Size());
		idIndices.Reserve(other.idIndices.Capacity());
		for (i32 i = 0; i < other.Size(); ++i)
		{
			const Id id = other.idList[i];
			if (id.GetVersion() != NoIdVersion)
			{
				EmplaceId(id, true);
			}
		}
	}

	ComponentPool::ComponentPool(ComponentPool&& other) noexcept
	    : IPool(other.typeId), idIndices{Move(other.idIndices)}, idList{Move(other.idList)}
	{
		BindOnPageAllocated();
		arena            = other.arena;
		lastRemovedIndex = Exchange(other.lastRemovedIndex, NO_INDEX);
		removePolicy     = other.removePolicy;
		typeId           = other.typeId;
	}

	ComponentPool& ComponentPool::operator=(const ComponentPool& other) noexcept
	{
		typeId       = other.typeId;
		idIndices    = {*other.arena};
		idList       = {*other.arena};
		arena        = other.arena;
		removePolicy = other.removePolicy;
		typeId       = other.typeId;
		BindOnPageAllocated();
		idList.Reserve(other.idList.Size());
		idIndices.Reserve(other.idIndices.Capacity());
		for (i32 i = 0; i < other.Size(); ++i)
		{
			const Id id = other.idList[i];
			if (id.GetVersion() != NoIdVersion)
			{
				EmplaceId(id, true);
			}
		}
		return *this;
	}

	ComponentPool& ComponentPool::operator=(ComponentPool&& other) noexcept
	{
		typeId           = other.typeId;
		idIndices        = Move(other.idIndices);
		idList           = Move(other.idList);
		arena            = other.arena;
		lastRemovedIndex = Exchange(other.lastRemovedIndex, NO_INDEX);
		removePolicy     = other.removePolicy;
		typeId           = other.typeId;
		return *this;
	}

	ComponentPool::Index ComponentPool::EmplaceId(const Id id, bool forceBack)
	{
		P_CheckMsg(!Has(id), "Set already contains entity");
		const auto idIndex = id.GetIndex();

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

	void ComponentPool::PopId(Id id)
	{
		const Index index = id.GetIndex();
		i32& idIndex      = idIndices[index];

		idList[idIndex]  = MakeId(index, NoIdVersion);    // Mark invalid but keep index
		lastRemovedIndex = idIndex;
		idIndex          = NO_INDEX;
	}

	void ComponentPool::PopSwapId(Id id)
	{
		i32& idIndex = idIndices[id.GetIndex()];
		idList.RemoveAtSwapUnsafe(idIndex);

		i32& lastIndex = idIndices[idList.Last().GetIndex()];
		// Move last element to current index
		idIndex   = lastIndex;
		lastIndex = NO_INDEX;
	}

	void ComponentPool::ClearIds()
	{
		if (lastRemovedIndex == NO_INDEX)
		{
			for (Id id : idList)
			{
				idIndices[id.GetIndex()] = NO_INDEX;
			}
		}
		else
		{
			for (Id id : idList)
			{
				if (id.GetVersion() != NoIdVersion)
				{
					idIndices[id.GetIndex()] = NO_INDEX;
				}
			}
		}

		lastRemovedIndex = NO_INDEX;
		idList.Clear();
	}

	void ComponentPool::BindOnPageAllocated()
	{
		idIndices.onPageAllocated = [](i32 index, i32* page, i32 size) {
			std::uninitialized_fill_n(page, size, NO_INDEX);
		};
	}

	TPool<CRemoved>::TPool(p::EntityContext& ctx, Arena& arena)
	    : IPool(p::GetTypeId<CRemoved>()), idRegistry{&ctx.GetIdRegistry()}
	{}

	i32 TPool<CRemoved>::Size() const
	{
		return idRegistry->GetDeferredRemovals().Size();
	}

	void TPool<CRemoved>::Add(Id id, CRemoved)
	{
		idRegistry->DeferredRemove(id);
	}

	const TArray<Id>& TPool<CRemoved>::GetIdList() const
	{
		return idRegistry->GetDeferredRemovals();
	}

	i32 GetSmallestPool(TView<const IPool* const> pools)
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


	PoolInstance::PoolInstance(TypeId componentId, TUniquePtr<IPool>&& pool)
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

	IPool* PoolInstance::GetPool() const
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

	void* EntityContext::AddDefault(TypeId typeId, Id id)
	{
		if (IPool* pool = GetPool(typeId))
		{
			return pool->AddDefault(id);
		}
		return nullptr;
	}

	void EntityContext::Remove(TypeId typeId, Id id)
	{
		if (IPool* pool = GetPool(typeId))
		{
			pool->Remove(id);
		}
	}

	IPool* EntityContext::GetPool(TypeId componentId) const
	{
		const i32 index = pools.FindSorted(PoolInstance{componentId, {}});
		return index != NO_INDEX ? pools[index].GetPool() : nullptr;
	}

	void EntityContext::GetPools(TView<const TypeId> componentIds, TArray<IPool*>& outPools) const
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
			pools.Add(Move(instance));
		}

		// TODO: Copy statics
		// TODO: Cache pools
	}

	void EntityContext::MoveFrom(EntityContext&& other)
	{
		idRegistry = Move(other.idRegistry);
		pools      = Move(other.pools);
		statics    = Move(other.statics);

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
		{
			*bAdded = true;
		}
		return statics[index];
	}


	void ExcludeIdsWith(const IPool* pool, TArray<Id>& ids, const bool shouldShrink)
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

	void ExcludeIdsWithStable(const IPool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ids.RemoveIf(
		    [pool](Id id) {
			return pool->Has(id);
		    },
		    shouldShrink);
	}

	void ExcludeIdsWithout(const IPool* pool, TArray<Id>& ids, const bool shouldShrink)
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

	void ExcludeIdsWithoutStable(const IPool* pool, TArray<Id>& ids, const bool shouldShrink)
	{
		ids.RemoveIf(
		    [pool](Id id) {
			return !pool->Has(id);
		    },
		    shouldShrink);
	}

	void FindIdsWith(const IPool* pool, TView<const Id> source, TArray<Id>& results)
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
	void FindIdsWith(TView<const IPool* const> pools, TView<const Id> source, TArray<Id>& results)
	{
		FindIdsWith(pools.First(), source, results);
		for (i32 i = 1; i < pools.Size(); ++i)
		{
			ExcludeIdsWithout(pools[i], results, false);
		}
	}

	void FindIdsWithout(const IPool* pool, TView<const Id> source, TArray<Id>& results)
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
	    const IPool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
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
	    const IPool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
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
	    const IPool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
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
	    const IPool* pool, TArray<Id>& source, TArray<Id>& results, const bool shouldShrink)
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

	void FindAllIdsWith(TView<const IPool* const> pools, TArray<Id>& ids)
	{
		if (pools.IsEmpty())
		{
			return;
		}

		for (const IPool* pool : pools)
		{
			if (!P_EnsureMsg(pool,
			        "One of the pools is null. Is the access missing one or more of the "
			        "specified "
			        "components?"))
			{
				return;
			}
		}

		const i32 smallestIdx     = GetSmallestPool(pools);
		const IPool* iterablePool = pools[smallestIdx];

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
				const IPool* pool = pools[i];
				ExcludeIdsWithout(pool, ids, false);
			}
		}
	}

	void FindAllIdsWithAny(TView<const IPool* const> pools, TArray<Id>& ids)
	{
		if (pools.IsEmpty())
		{
			return;
		}

		for (const IPool* pool : pools)
		{
			if (!P_EnsureMsg(pool,
			        "One of the pools is null. Is the access missing one or more of the "
			        "specified "
			        "components?"))
			{
				return;
			}
		}

		ids.Clear();
		for (const IPool* pool : pools)
		{
			ids.Append(pool->begin(), pool->end());
		}
	}

	void FindAllIdsWithAnyUnique(TView<const IPool* const> pools, TArray<Id>& ids)
	{
		if (pools.IsEmpty())
		{
			return;
		}

		i32 maxPossibleSize = 0;
		for (const IPool* pool : pools)
		{
			if (!P_EnsureMsg(pool,
			        "One of the pools is null. Is the access missing one or more of the "
			        "specified "
			        "components?"))
			{
				return;
			}

			maxPossibleSize += pool->Size();
		}

		TSet<Id> idsSet;
		idsSet.Reserve(maxPossibleSize);
		for (const IPool* pool : pools)
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


	Id AddId(EntityContext& ctx)
	{
		return ctx.GetIdRegistry().Create();
	}
	void AddId(EntityContext& ctx, TView<Id> ids)
	{
		ctx.GetIdRegistry().Create(ids);
	}

	bool RmId(EntityContext& ctx, TView<const Id> ids, RmIdFlags flags)
	{
		TArray<Id> allIds;    // Only used when removing children. Here for scope purposes.
		if (HasFlag(flags, p::RmIdFlags::RemoveChildren))
		{
			allIds.Append(ids);
			GetAllIdChildren(ctx, ids, allIds);
			// No children to detach since we will remove all of them
			ids = allIds;
		}

		if (HasFlag(flags, p::RmIdFlags::Instant))
		{
			for (auto& pool : ctx.GetPools())
			{
				pool.GetPool()->Remove(ids);
			}
			return ctx.GetIdRegistry().Remove(ids);
		}
		else
		{
			return ctx.GetIdRegistry().DeferredRemove(ids);
		}
	}

	bool FlushDeferredRemovals(EntityContext& ctx)
	{
		TView<Id> ids = ctx.GetIdRegistry().GetDeferredRemovals();
		for (auto& pool : ctx.GetPools())
		{
			pool.GetPool()->Remove(ids);
		}
		return ctx.GetIdRegistry().FlushDeferredRemovals();
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
						access.Get<CChild>(child).parent = NoId;
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
