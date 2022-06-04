// Copyright 2015-2022 Piperift - All rights reserved

#include "Files/STDFileSystem.h"

#include "Files/Paths.h"
#include "Serialization/Contexts.h"


void Read(Pipe::ReadContext& ct, Pipe::Path& value)
{
	Pipe::String str;
	ct.Serialize(str);
	value = Pipe::FromString(str);
}

void Write(Pipe::WriteContext& ct, const Pipe::Path& value)
{
	ct.Serialize(Pipe::ToString(value));
}
