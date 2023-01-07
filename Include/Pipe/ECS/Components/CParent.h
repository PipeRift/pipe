// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/ECS/Id.h"
#include "Pipe/Reflect/Struct.h"


namespace p::ecs
{
	struct PIPE_API CParent : public Struct
	{
		STRUCT(CParent, Struct)

		PROP(children)
		TArray<Id> children;
	};

	static void Read(Reader& ct, CParent& val)
	{
		ct.Serialize(val.children);
	}
	static void Write(Writer& ct, const CParent& val)
	{
		ct.Serialize(val.children);
	}
}    // namespace p::ecs
