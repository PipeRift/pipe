// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Export.h"
#include "Files/STDFileSystem.h"
#include "Platform/Platform.h"
#include "Strings/String.h"
#include "Strings/StringView.h"


namespace Rift::Paths
{
	///////////////////////////////////////////////////////////
	// PATHS

	inline CORE_API void SetCurrent(Path path)
	{
		fs::current_path(path);
	}

	inline CORE_API Path GetCurrent()
	{
		return fs::current_path();
	}

	CORE_API Path GetBasePath();


	///////////////////////////////////////////////////////////
	// PATH HELPERS

	inline CORE_API constexpr bool IsSlash(TChar c)
	{
		return c == TX('\\') || c == TX('/');
	}

	CORE_API const TChar* FindRelativeChar(const TChar* const first, const TChar* const last);

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


	CORE_API Path ToRelative(const Path& path, const Path& parent = GetCurrent());
	CORE_API Path ToAbsolute(const Path& path, const Path& parent = GetCurrent());
	CORE_API bool IsInside(const Path& base, const Path& parent);
	CORE_API String GetFilename(const Path& path);
	CORE_API String ToString(const Path& path);
	CORE_API Path FromString(StringView pathStr);
}    // namespace Rift::Paths
