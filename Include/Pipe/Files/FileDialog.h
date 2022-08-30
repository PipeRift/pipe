// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Tuples.h"
#include "Pipe/Files/STDFileSystem.h"


namespace p::files
{
	using DialogFileFilter = TPair<StringView, StringView>;

	/**
	 * Selects a file using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath where to open the dialog
	 * @param alwaysShowDefaultPath if true dialog will open to default path. Else, the system
	 * can decide to, for example, display last user folder (windows).
	 * @return selected file path
	 */
	PIPE_API Path SelectFileDialog(StringView title, const Path& defaultPath,
	    const TArray<DialogFileFilter>& filters =
	        {
	            {"All Files", "*"}
    },
	    bool alwaysShowDefaultPath = false);

	/**
	 * Selects multiple files using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath where to open the dialog
	 * @param outFiles selected by the user
	 * @param alwaysShowDefaultPath if true dialog will open to default path. Else, the system
	 * can decide to, for example, display last user folder (windows).
	 */
	PIPE_API void SelectFilesDialog(StringView title, const Path& defaultPath,
	    TArray<Path>& outFiles,
	    const TArray<DialogFileFilter>& filters =
	        {
	            {"All Files", "*"}
    },
	    bool alwaysShowDefaultPath = false);

	/**
	 * Selects a folder using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath where to open the dialog
	 * @param bAlwaysShowDefaultPath if true dialog will open to default path. Else, the system
	 * can decide to, for example, display last user folder (windows).
	 * @return selected folder path
	 */
	PIPE_API Path SelectFolderDialog(
	    StringView title, const Path& defaultPath, bool alwaysShowDefaultPath = false);

	/**
	 * Selects a file path for a file to be saved using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath where to open the dialog
	 * @param alwaysShowDefaultPath if true dialog will open to default path. Else, the system
	 * can decide to, for example, display last user folder (windows).
	 * @return selected file path
	 */
	PIPE_API Path SaveFileDialog(StringView title, const Path& defaultPath,
	    const TArray<DialogFileFilter>& filters =
	        {
	            {"All Files", "*"}
    },
	    bool alwaysShowDefaultPath = false, bool confirmOverwrite = false);
}    // namespace p::files

namespace p
{
	using namespace p::files;
}
