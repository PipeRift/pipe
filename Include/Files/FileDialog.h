// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Containers/Tuples.h"
#include "Files/FileSystem.h"
#include "Strings/String.h"


namespace Rift::Dialogs
{
	using FileFilter = TPair<StringView, StringView>;

	/**
	 * Selects a file using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath where to open the dialog
	 * @param bAlwaysShowDefaultPath if true dialog will open to default path. Else, the system
	 * can decide to, for example, display last user folder (windows).
	 * @return selected file path
	 */
	CORE_API Path SelectFile(StringView title, const Path& defaultPath,
	    const TArray<FileFilter>& filters = {{"All Files", "*"}},
	    bool bAlwaysShowDefaultPath       = false);

	/**
	 * Selects multiple files using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath where to open the dialog
	 * @param outFiles selected by the user
	 * @param bAlwaysShowDefaultPath if true dialog will open to default path. Else, the system
	 * can decide to, for example, display last user folder (windows).
	 */
	CORE_API void SelectFiles(StringView title, const Path& defaultPath, TArray<Path>& outFiles,
	    const TArray<FileFilter>& filters = {{"All Files", "*"}},
	    bool bAlwaysShowDefaultPath       = false);

	/**
	 * Selects a folder using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath where to open the dialog
	 * @param bAlwaysShowDefaultPath if true dialog will open to default path. Else, the system
	 * can decide to, for example, display last user folder (windows).
	 * @return selected folder path
	 */
	CORE_API Path SelectFolder(
	    StringView title, const Path& defaultPath, bool bAlwaysShowDefaultPath = false);
}    // namespace Rift::Dialogs
