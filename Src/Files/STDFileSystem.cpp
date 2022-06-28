// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Files/STDFileSystem.h"

#include "Pipe/Files/Paths.h"
#include "Pipe/Serialize/Serialization.h"


namespace p
{
	void Read(p::Reader& ct, p::Path& value)
	{
		p::String str;
		ct.Serialize(str);
		value = p::ToPath(str);
	}

	void Write(p::Writer& ct, const p::Path& value)
	{
		ct.Serialize(p::ToString(value));
	}
}    // namespace p
