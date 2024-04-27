// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Files/STDFileSystem.h"

#include "Pipe/Files/Files.h"
#include "Pipe/Files/Paths.h"
#include "Pipe/Serialize/Serialization.h"


namespace p
{
	void Read(p::Reader& ct, p::Path& value)
	{
		p::String str;
		ct.Serialize(str);
		value = ToSTDPath(str);
	}

	void Write(p::Writer& ct, const p::Path& value)
	{
		ct.Serialize(ToString(value));
	}

	PathIterator CreatePathIterator(StringView path)
	{
		if (!IsFolder(path))
		{
			return {};
		}
		return PathIterator{path};
	}

	RecursivePathIterator CreateRecursivePathIterator(StringView path)
	{
		if (!Exists(path) || !IsFolder(path))
		{
			return {};
		}
		return RecursivePathIterator{path};
	}
}    // namespace p
