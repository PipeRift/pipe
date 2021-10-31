// Copyright 2015-2021 Piperift - All rights reserved

#include "Files/STDFileSystem.h"

#include "Files/Paths.h"
#include "Serialization/Contexts.h"


namespace Rift::Serl
{
	void Read(Serl::ReadContext& ct, Path& value)
	{
		String str;
		ct.Serialize(str);
		value = Paths::FromString(str);
	}

	void Write(Serl::WriteContext& ct, const Path& value)
	{
		ct.Serialize(Paths::ToString(value));
	}
}    // namespace Rift::Serl
