// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Core/Platform.h"
#include "Core/String.h"
#include "Core/StringView.h"
#include "Export.h"
#include "Files/STDFileSystem.h"


namespace pipe::Files
{
	///////////////////////////////////////////////////////////
	// PATHS

	inline CORE_API void SetCurrentPath(Path path)
	{
		fs::current_path(path);
	}

	inline CORE_API Path GetCurrentPath()
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
	CORE_API const TChar* FindFilename(const TChar* const first, const TChar* last);

	// @return root name of a path, or an empty view if missing
	// E.g: "C:\Folder" -> "C:"
	CORE_API StringView GetRootPathName(const StringView path);

	// @return root path of a path, or an empty view if missing
	// E.g: "C:\Folder" -> "C:\"
	CORE_API StringView GetRootPath(const StringView path);

	// @return the relative path if it exists, otherwise, an empty view
	// E.g: "C:\Folder\Other" -> "Folder\Other"
	CORE_API StringView GetRelativePath(const StringView path);

	// @return the path to the parent directory
	// E.g: "/var/tmp/file.txt" -> "/var/tmp"
	// E.g: "/var/tmp/." -> "/var/tmp"
	CORE_API StringView GetParentPath(StringView path);

	CORE_API StringView GetFilename(StringView path);
	CORE_API String GetFilename(const Path& path);
	inline StringView GetFilename(const String& path)
	{
		return GetFilename(StringView{path});
	}


	CORE_API Path ToRelativePath(const Path& path, const Path& parent = GetCurrentPath());
	CORE_API Path ToAbsolutePath(const Path& path, const Path& parent = GetCurrentPath());
	CORE_API bool IsRelativePath(const Path& path);
	CORE_API bool IsAbsolutePath(const Path& path);
	CORE_API String ToString(const Path& path);
	CORE_API Path FromString(StringView pathStr);
}    // namespace pipe::Files

namespace pipe
{
	using namespace pipe::Files;
}
