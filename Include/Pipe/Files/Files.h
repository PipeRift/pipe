// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Files/STDFileSystem.h"
#include "PipeArrays.h"


namespace p::files
{
	/** Path API */

	PIPE_API bool ExistsAsFile(const Path& path);
	PIPE_API bool ExistsAsFolder(const Path& path);
	PIPE_API SpaceInfo Space(const Path& target);


	/** String API */

	PIPE_API bool CreateFolder(StringView path, bool bRecursive = false);
	PIPE_API bool Delete(StringView path, bool bExcludeIfNotEmpty = true, bool bLogErrors = true);

	// TODO: Implement plural copy and move
	PIPE_API bool Copy(StringView origin, StringView destination,
	    CopyOptions options = CopyOptions::Overwrite | CopyOptions::Recursive);
	PIPE_API bool Move(StringView origin, StringView destination);
	PIPE_API bool Rename(StringView origin, StringView destination);

	PIPE_API bool LoadStringFile(StringView path, String& result, sizet extraPadding = 0);
	PIPE_API bool SaveStringFile(StringView path, StringView data);

	PIPE_API bool Exists(StringView path);
	PIPE_API bool IsFolder(StringView path);
	PIPE_API bool IsFile(StringView path);
}    // namespace p::files

namespace p
{
	using namespace p::files;
}
