// Copyright 2015-2022 Piperift - All rights reserved

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
			const Id entity = entities[i];
			if (entity != NoId)
			{
				ids[i] = entity;
			}
			else
			{
				ids[i] = context.Create();
			}
		}
		// Create all non-root entities
		context.Create({ids.Data() + maxSize, ids.Data() + ids.Size()});

		if (EnterNext("components"))
		{
			BeginObject();
			onReadPools(*this);
			Leave();
		}

		p::ecs::FixParentLinks(context, parents);
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

		const i32 idCount = ids.Size();
		Next("count", idCount);

		idToIndexes.Reserve(idCount);
		for (i32 i = 0; i < i32(idCount); ++i)
		{
			idToIndexes.Insert(ids[i], i);
		}

		Next("roots", entities);
		if (EnterNext("components"))
		{
			BeginObject();
			onWritePools(*this);
			Leave();
		}
	}

	void EntityWriter::RetrieveHierarchy(const TArray<Id>& roots, TArray<Id>& children)
	{
		children.Append(roots);

		if (includeChildren)
		{
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
}    // namespace p::ecs
