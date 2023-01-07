// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/ECS/Serialization.h"

#include "Pipe/ECS/Components/CNotSerialized.h"
#include "Pipe/ECS/Utils/Hierarchy.h"


namespace p::ecs
{
	void EntityReader::SerializeEntities(
	    TArray<Id>& entities, TFunction<void(EntityReader&)> onReadPools)
	{
		TArray<Id> parents;
		p::ecs::GetParents(context, entities, parents);

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

		p::ecs::FixParentLinks(context, parents);
	}

	void EntityReader::SerializeSingleEntity(Id& entity, TFunction<void(EntityReader&)> onReadPools)
	{
		Id parent = p::ecs::GetParent(context, entity);
		if (entity == NoId)
		{
			entity = context.Create();
		}
		ids.Assign(&entity, 1);

		onReadPools(*this);

		p::ecs::FixParentLinks(context, parent);
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

	void EntityWriter::SerializeSingleEntity(Id entity, TFunction<void(EntityWriter&)> onWritePools)
	{
		ids.Assign(&entity, 1);
		MapIdsToIndices();

		onWritePools(*this);
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
			p::ecs::GetChildren(context, pendingInspection, currentLinked);
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
}    // namespace p::ecs
