// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Export.h"
#include "Pipe/Files/STDFileSystem.h"


namespace p::files
{
	///////////////////////////////////////////////////////////
	// PATHS

	inline PIPE_API void SetCurrentPath(Path path)
	{
		fs::current_path(path);
	}

	inline PIPE_API Path GetCurrentPath()
	{
		return fs::current_path();
	}

	PIPE_API Path GetBasePath();


	///////////////////////////////////////////////////////////
	// PATH HELPERS

	inline PIPE_API constexpr bool IsSlash(TChar c)
	{
		return c == TX('\\') || c == TX('/');
	}

	PIPE_API const TChar* FindRelativeChar(const TChar* const first, const TChar* const last);
	PIPE_API const TChar* FindFilename(const TChar* const first, const TChar* last);
	PIPE_API const TChar* FindExtension(const TChar* const first, const TChar* last);

	// @return root name of a path, or an empty view if missing
	// E.g: "C:\Folder" -> "C:"
	PIPE_API StringView GetRootPathName(const StringView path);

	// @return root path of a path, or an empty view if missing
	// E.g: "C:\Folder" -> "C:\"
	PIPE_API StringView GetRootPath(const StringView path);

	// @return the relative path if it exists, otherwise, an empty view
	// E.g: "C:\Folder\Other" -> "Folder\Other"
	PIPE_API StringView GetRelativePath(const StringView path);

	// @return the path to the parent directory
	// E.g: "/var/tmp/file.txt" -> "/var/tmp"
	// E.g: "/var/tmp/." -> "/var/tmp"
	PIPE_API StringView GetParentPath(StringView path);

	PIPE_API StringView GetFilename(StringView path);
	inline StringView GetFilename(const String& path)
	{
		return GetFilename(StringView{path});
	}
	PIPE_API bool HasFilename(StringView path);

	PIPE_API StringView GetStem(StringView path);
	PIPE_API bool HasStem(StringView path);

	PIPE_API StringView GetExtension(StringView path);
	PIPE_API bool HasExtension(StringView path);


	PIPE_API String GetFilename(const Path& path);
	PIPE_API Path ToRelativePath(const Path& path, const Path& parent = GetCurrentPath());
	PIPE_API Path ToAbsolutePath(const Path& path, const Path& parent = GetCurrentPath());
	PIPE_API bool IsRelativePath(const Path& path);
	PIPE_API bool IsAbsolutePath(const Path& path);
	PIPE_API String ToString(const Path& path);
	PIPE_API Path ToPath(StringView pathStr);
}    // namespace p::files

namespace p
{
	using namespace p::files;
}
