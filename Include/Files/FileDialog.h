// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "PCH.h"

#include "Files/FileSystem.h"


namespace Rift
{
	struct FileDialog
	{
		FileDialog() {}
	};

	/**
	 * Selects a file using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath path where to open the dialog
	 * @param bAlwaysShowDefaultPath if true dialog will open to default path. Else, the system can
	 * decide to for example display last user folder (windows).
	 */
	static Path SelectFile(StringView title, Path defaultPath, bool bAlwaysShowDefaultPath = false);

	/**
	 * Selects multiple files using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath path where to open the dialog
	 * @param outFiles selected by the user
	 * @param bAlwaysShowDefaultPath if true dialog will open to default path. Else, the system can
	 * decide to for example display last user folder (windows).
	 */
	static void SelectFiles(StringView title, Path defaultPath, TArray<Path>& outFiles,
	    bool bAlwaysShowDefaultPath = false);

	/**
	 * Selects a file using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath path where to open the dialog
	 * @param bAlwaysShowDefaultPath if true dialog will open to default path. Else, the system can
	 * decide to for example display last user folder (windows).
	 */
	static Path SelectFolder(
	    StringView title, Path defaultPath, bool bAlwaysShowDefaultPath = false);

	/**
	 * Selects multiple files using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath path where to open the dialog
	 * @param outFiles selected by the user
	 * @param bAlwaysShowDefaultPath if true dialog will open to default path. Else, the system can
	 * decide to for example display last user folder (windows).
	 */
	static void SelectFolders(StringView title, Path defaultPath, TArray<Path>& outFiles,
	    bool bAlwaysShowDefaultPath = false);

	/**
	 * Selects a file using a system dialog.
	 * @param title of the dialog
	 * @param callback the selected file path
	 */
	// static void SelectFile(StringView title, TFunction<String()> callback)
}    // namespace Rift
