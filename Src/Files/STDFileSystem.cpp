// Copyright 2015-2022 Piperift - All rights reserved

#include "Files/STDFileSystem.h"

#include "Files/Paths.h"
#include "Serialization/Contexts.h"


namespace p::serl
{
	void Read(p::ReadContext& ct, p::Path& value)
	{
		p::String str;
		ct.Serialize(str);
		value = p::FromString(str);
	}

	void Write(p::WriteContext& ct, const p::Path& value)
	{
		ct.Serialize(p::ToString(value));
	}
}    // namespace p::serl
