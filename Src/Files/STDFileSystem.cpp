// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Files/STDFileSystem.h"

#include "Pipe/Files/Files.h"
#include "Pipe/Files/Paths.h"
#include "PipeSerialize.h"


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

	DirectoryIterator CreateDirIterator(StringView path)
	{
		if (!IsFolder(path))
		{
			return {};
		}
		return DirectoryIterator{path};
	}

	RecursiveDirectoryIterator CreateRecursiveDirIterator(StringView path)
	{
		if (!Exists(path) || !IsFolder(path))
		{
			return {};
		}
		return RecursiveDirectoryIterator{path};
	}
}    // namespace p
