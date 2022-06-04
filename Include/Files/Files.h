// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Files/STDFileSystem.h"
#include "Strings/String.h"


namespace Pipe::Files
{
	using Iterator          = fs::directory_iterator;
	using RecursiveIterator = fs::recursive_directory_iterator;
	using SpaceInfo         = fs::space_info;


	/** Path API */

	CORE_API bool LoadStringFile(const Path& path, String& result, sizet extraPadding = 0);
	CORE_API bool SaveStringFile(const Path& path, StringView data);

	CORE_API void CreateFolder(const Path& path, bool bRecursive = false);
	CORE_API bool Delete(const Path& path, bool bExcludeIfNotEmpty = true, bool bLogErrors = true);

	CORE_API Iterator CreateIterator(const Path& path);
	CORE_API RecursiveIterator CreateRecursiveIterator(const Path& path);


	/** Helpers */

	CORE_API bool Exists(const Path& path);
	CORE_API bool IsFolder(const Path& path);
	CORE_API bool IsFile(const Path& path, bool /*bCheckOnDisk*/ = true);
	CORE_API bool ExistsAsFile(const Path& path);
	CORE_API bool ExistsAsFolder(const Path& path);
	CORE_API SpaceInfo Space(const Path& target);

	// TODO: Implement plural copy and move
	CORE_API bool Copy(const Path& origin, const Path& destination,
	    CopyOptions options = CopyOptions::Overwrite | CopyOptions::Recursive);
	CORE_API bool Move(const Path& origin, const Path& destination);
	CORE_API bool Rename(const Path& origin, const Path& destination);


	/** String API */

	CORE_API bool LoadStringFile(const String& path, String& result, sizet extraPadding = 0);
	CORE_API bool SaveStringFile(const String& path, StringView data);

	CORE_API bool Exists(const String& path);
	CORE_API bool IsFolder(const String& path);
	CORE_API bool IsFile(const String& path);
}    // namespace Pipe::Files

namespace Pipe
{
	using namespace Pipe::Files;
}
