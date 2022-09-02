// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/ECS/Id.h"
#include "Pipe/Reflect/Struct.h"


namespace p::ecs
{
	struct PIPE_API CChild : public Struct
	{
		STRUCT(CChild, Struct)

		PROP(parent)
		Id parent = NoId;

		CChild() = default;
		CChild(Id parent) : parent(parent) {}
	};

	static void Read(Reader& ct, CChild& val)
	{
		ct.Serialize(val.parent);
	}
	static void Write(Writer& ct, const CChild& val)
	{
		ct.Serialize(val.parent);
	}
}    // namespace p::ecs
