// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/PipeECS.h"

#include "Pipe/ECS/Serialization.h"


namespace p::ecs
{
	void Read(Reader& r, Id& val)
	{
		auto* entityReader = dynamic_cast<p::ecs::EntityReader*>(&r);
		if (EnsureMsg(entityReader, "Serializing an ecs Id without an EntityReader")) [[likely]]
		{
			i32 dataId;
			entityReader->Serialize(dataId);
			val = dataId >= 0 ? entityReader->GetIds()[dataId] : NoId;
		}
	}

	void Write(Writer& w, Id val)
	{
		auto* entityWriter = dynamic_cast<p::ecs::EntityWriter*>(&w);
		if (EnsureMsg(entityWriter, "Serializing an ecs Id without an EntityWriter")) [[likely]]
		{
			const i32* dataId = entityWriter->GetIdToIndexes().Find(val);
			entityWriter->Serialize(dataId ? *dataId : -1);
		}
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
		const Index index = GetIndex(id);
		if (entities.IsValidIndex(index))
		{
			Id& storedId = entities[index];
			if (id == storedId)
			{
				// Increase version to invalidate current entity
				storedId = MakeId(index, GetVersion(storedId) + 1u);
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
			const Index index = GetIndex(id);
			if (entities.IsValidIndex(index))
			{
				Id& storedId = entities[index];
				if (id == storedId)
				{
					// Increase version to invalidate current entity
					storedId = MakeId(index, GetVersion(storedId) + 1u);
					available.Add(index);
				}
			}
		}
		return (available.Size() - lastAvailable) > 0;
	}

	bool IdRegistry::IsValid(Id id) const
	{
		const Index index = GetIndex(id);
		return entities.IsValidIndex(index) && entities[index] == id;
	}
}    // namespace p::ecs
