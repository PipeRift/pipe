// Copyright 2015-2022 Piperift - All rights reserved

#include "Files/STDFileSystem.h"

#include "Files/Paths.h"
#include "Serialization/Contexts.h"


namespace Pipe::Serl
{
	void Read(Serl::ReadContext& ct, Path& value)
	{
		String str;
		ct.Serialize(str);
		value = FromString(str);
	}

	void Write(Serl::WriteContext& ct, const Path& value)
	{
		ct.Serialize(ToString(value));
	}
}    // namespace Pipe::Serl
