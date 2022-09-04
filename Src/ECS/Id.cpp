// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/ECS/Id.h"

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
}    // namespace p::ecs
