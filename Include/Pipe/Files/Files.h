// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/String.h"
#include "Pipe/Files/STDFileSystem.h"


namespace p::files
{
	using Iterator          = fs::directory_iterator;
	using RecursiveIterator = fs::recursive_directory_iterator;
	using SpaceInfo         = fs::space_info;


	/** Path API */

	PIPE_API bool LoadStringFile(const Path& path, String& result, sizet extraPadding = 0);
	PIPE_API bool SaveStringFile(const Path& path, StringView data);

	PIPE_API void CreateFolder(const Path& path, bool bRecursive = false);
	PIPE_API bool Delete(const Path& path, bool bExcludeIfNotEmpty = true, bool bLogErrors = true);

	PIPE_API Iterator CreateIterator(const Path& path);
	PIPE_API RecursiveIterator CreateRecursiveIterator(const Path& path);


	/** Helpers */

	PIPE_API bool Exists(const Path& path);
	PIPE_API bool IsFolder(const Path& path);
	PIPE_API bool IsFile(const Path& path, bool /*bCheckOnDisk*/ = true);
	PIPE_API bool ExistsAsFile(const Path& path);
	PIPE_API bool ExistsAsFolder(const Path& path);
	PIPE_API SpaceInfo Space(const Path& target);

	// TODO: Implement plural copy and move
	PIPE_API bool Copy(const Path& origin, const Path& destination,
	    CopyOptions options = CopyOptions::Overwrite | CopyOptions::Recursive);
	PIPE_API bool Move(const Path& origin, const Path& destination);
	PIPE_API bool Rename(const Path& origin, const Path& destination);


	/** String API */

	PIPE_API bool LoadStringFile(const String& path, String& result, sizet extraPadding = 0);
	PIPE_API bool SaveStringFile(const String& path, StringView data);

	PIPE_API bool Exists(const String& path);
	PIPE_API bool IsFolder(const String& path);
	PIPE_API bool IsFile(const String& path);
}    // namespace p::files

namespace p
{
	using namespace p::files;
}
