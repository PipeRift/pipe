// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Files/STDFileSystem.h"
#include "PipeArrays.h"


namespace p
{
	/** Path API */

	P_API bool ExistsAsFile(const Path& path);
	P_API bool ExistsAsFolder(const Path& path);
	P_API SpaceInfo Space(const Path& target);


	/** String API */

	P_API bool CreateFolder(StringView path, bool bRecursive = false);
	P_API bool Delete(StringView path, bool bExcludeIfNotEmpty = true, bool bLogErrors = true);

	// TODO: Implement plural copy and move
	P_API bool Copy(StringView origin, StringView destination,
	    CopyOptions options = CopyOptions::Overwrite | CopyOptions::Recursive);
	P_API bool Move(StringView origin, StringView destination);
	P_API bool Rename(StringView origin, StringView destination);

	P_API bool LoadStringFile(StringView path, String& result, sizet extraPadding = 0);
	P_API bool SaveStringFile(StringView path, StringView data);

	P_API bool Exists(StringView path);
	P_API bool IsFolder(StringView path);
	P_API bool IsFile(StringView path);
}    // namespace p
