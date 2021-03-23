// Copyright 2015-2021 Piperift - All rights reserved

#include "Export.h"
#include "Platform/Platform.h"
#include "Strings/StringView.h"


namespace Rift::Paths
{
	inline CORE_API constexpr bool IsSlash(TCHAR c)
	{
		return c == TX('\\') || c == TX('/');
	}

	CORE_API const TCHAR* FindRelativeChar(const TCHAR* const first, const TCHAR* const last);

	// @return root name of a path, or an empty view if missing
	// E.g: "C:\Folder" -> "C:"
	CORE_API StringView GetRootName(const StringView path);

	// @return root path of a path, or an empty view if missing
	// E.g: "C:\Folder" -> "C:\"
	CORE_API StringView GetRoot(const StringView path);

	// @return the relative path if it exists, otherwise, an empty view
	// E.g: "C:\Folder\Other" -> "Folder\Other"
	CORE_API StringView GetRelative(const StringView path);

	// @return the path to the parent directory
	// E.g: "/var/tmp/file.txt" -> "/var/tmp"
	// E.g: "/var/tmp/." -> "/var/tmp"
	CORE_API StringView GetParent(StringView path);
}    // namespace Rift::Paths
