// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Templates.h"
#include "Pipe/Files/Paths.h"
#include "Pipe/Memory/OwnPtr.h"


namespace p
{
#pragma region FileWatch
	enum class FileWatchAction : p::u8
	{
		Add      = 1,
		Delete   = 2,
		Modified = 3,
		Moved    = 4
	};

	using FileListenerId    = long;
	using FileWatchCallback = std::function<void(
	    StringView path, StringView filename, FileWatchAction action, StringView oldFilename)>;

	struct PIPE_API FileWatcher
	{
	private:
		OwnPtr fileWatcher;
		p::TArray<TOwnPtr<struct FileWatchListener>> listeners;

	public:
		FileWatcher();
		~FileWatcher();
		FileWatcher(const FileWatcher& other)            = delete;
		FileWatcher& operator=(const FileWatcher& other) = delete;

		FileListenerId ListenPath(StringView path, bool recursive, FileWatchCallback callback);
		void StopListening(FileListenerId id);
		void Reset();

		void StartAsync();
	};
#pragma endregion FileWatch


#pragma region FileDialogs
	using DialogFileFilter = TPair<StringView, StringView>;

	/**
	 * Selects a file using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath where to open the dialog
	 * @param alwaysShowDefaultPath if true dialog will open to default path. Else, the system
	 * can decide to, for example, display last user folder (windows).
	 * @return selected file path
	 */
	PIPE_API String SelectFileDialog(StringView title, StringView defaultPath,
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
	PIPE_API void SelectFilesDialog(StringView title, StringView defaultPath,
	    TArray<String>& outFiles,
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
	PIPE_API String SelectFolderDialog(
	    StringView title, StringView defaultPath, bool alwaysShowDefaultPath = false);

	/**
	 * Selects a file path for a file to be saved using a system dialog.
	 * @param title of the dialog
	 * @param defaultPath where to open the dialog
	 * @param alwaysShowDefaultPath if true dialog will open to default path. Else, the system
	 * can decide to, for example, display last user folder (windows).
	 * @return selected file path
	 */
	PIPE_API String SaveFileDialog(StringView title, StringView defaultPath,
	    const TArray<DialogFileFilter>& filters =
	        {
	            {"All Files", "*"}
    },
	    bool alwaysShowDefaultPath = false, bool confirmOverwrite = false);
#pragma endregion FileDialogs
};    // namespace p
