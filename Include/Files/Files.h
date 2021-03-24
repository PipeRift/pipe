// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Containers/Array.h"
#include "Files/STDFileSystem.h"
#include "Serialization/Json.h"
#include "Strings/String.h"


namespace Rift::Files
{
	using Iterator          = fs::directory_iterator;
	using RecursiveIterator = fs::recursive_directory_iterator;
	using SpaceInfo         = fs::space_info;


	/** Path API */

	CORE_API bool LoadJsonFile(const Path& path, Json& result);
	CORE_API bool SaveJsonFile(const Path& path, const Json& data, i32 indent = -1);

	CORE_API bool LoadStringFile(const Path& path, String& result);
	CORE_API bool SaveStringFile(const Path& path, const String& data);

	CORE_API void CreateFolder(const Path& path, bool bRecursive = false);
	CORE_API bool Delete(const Path& path, bool bRemoveIfNotEmpty = true, bool bLogErrors = true);

	CORE_API Iterator CreateIterator(const Path& path);
	CORE_API RecursiveIterator CreateRecursiveIterator(const Path& path);


	/** Helpers */

	CORE_API bool Exists(const Path& path);
	CORE_API bool IsFolder(const Path& path);
	CORE_API bool IsFile(const Path& path, bool /*bCheckOnDisk*/ = true);
	CORE_API bool ExistsAsFile(const Path& path);
	CORE_API bool ExistsAsFolder(const Path& path);
	CORE_API SpaceInfo Space(const Path& target);


	/** String API */

	CORE_API bool LoadJsonFile(const String& path, Json& result);
	CORE_API bool SaveJsonFile(const String& path, const Json& data, i32 indent = -1);
	CORE_API bool LoadStringFile(const String& path, String& result);
	CORE_API bool SaveStringFile(const String& path, const String& data);

	CORE_API bool Exists(const String& path);
	CORE_API bool IsFolder(const String& path);
	CORE_API bool IsFile(const String& path);
}    // namespace Rift::Files
