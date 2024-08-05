// Copyright 2015-2024 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/StringView.h"
#include "Pipe/Core/Templates.h"
#include "Pipe/Files/Paths.h"
#include "Pipe/Memory/OwnPtr.h"


namespace p
{
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


#pragma region FileWatch
	enum class FileWatchAction : p::u8
	{
		Add      = 1,
		Delete   = 2,
		Modified = 3,
		Moved    = 4
	};

	using FileWatchId       = i32;
	using FileWatchCallback = std::function<void(FileWatchId id, StringView path,
	    StringView filename, FileWatchAction action, StringView oldFilename)>;

	struct PIPE_API FileWatcher
	{
	public:
		/** Should recursive watchers follow symbolic links? Default: false */
		bool followsSymlinks = false;

		/** Allow symlinks to watch recursively out of the pointed directory. Default: false.
		 * 'followsSymlinks' must be enabled.
		 * E.g: A symlink from '/home/folder' to '/'. With 'followsSymlinks=false' only '/home' and
		 * deeper are allowed. Set to false it will prevent infinite recursion.
		 */
		bool allowsOutOfScopeLinks = false;

	private:
		OwnPtr fileWatcher;

	public:
		FileWatcher(bool useGeneric = false);
		~FileWatcher();
		FileWatcher(const FileWatcher& other)            = delete;
		FileWatcher& operator=(const FileWatcher& other) = delete;

		FileWatchId ListenPath(StringView path, bool recursive, FileWatchCallback callback);
		void StopListening(FileWatchId id);
		void Reset();

		void StartWatchingAsync();
	};
#pragma endregion FileWatch
};    // namespace p
