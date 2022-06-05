// Copyright 2015-2022 Piperift - All rights reserved

#include "Files/STDFileSystem.h"

#include "Files/Paths.h"
#include "Serialization/Contexts.h"


namespace pipe::serl
{
	void Read(pipe::ReadContext& ct, pipe::Path& value)
	{
		pipe::String str;
		ct.Serialize(str);
		value = pipe::FromString(str);
	}

	void Write(pipe::WriteContext& ct, const pipe::Path& value)
	{
		ct.Serialize(pipe::ToString(value));
	}
}    // namespace pipe::serl
