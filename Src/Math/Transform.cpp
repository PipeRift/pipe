// Copyright 2015-2021 Piperift - All rights reserved

#include "Math/Transform.h"

#include "Math/Math.h"
#include "Serialization/Contexts.h"


namespace Rift
{
	void Transform::Read(Serl::ReadContext& ct)
	{
		ct.BeginObject();
		ct.Next("location", location);
		ct.Next("rotation", rotation);
		ct.Next("scale", scale);
	}

	void Transform::Write(Serl::WriteContext& ct) const
	{
		ct.BeginObject();
		ct.Next("location", location);
		ct.Next("rotation", rotation);
		ct.Next("scale", scale);
	}
}    // namespace Rift
