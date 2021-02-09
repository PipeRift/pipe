// Copyright 2015-2021 Piperift - All rights reserved

#include "Math/Transform.h"

#include "Math/Math.h"
#include "Serialization/Archive.h"


namespace Rift
{
	bool Transform::Serialize(Archive& ar, const char* name)
	{
		ar.BeginObject(name);
		{
			ar("location", location);
			ar("rotation", rotation);
			ar("scale", scale);
		}
		ar.EndObject();
		return true;
	}
}	 // namespace Rift