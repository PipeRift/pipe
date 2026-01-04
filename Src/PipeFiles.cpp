// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeFiles.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/Map.h"
#include "Pipe/Core/Tag.h"
#include "PipePlatform.h"

#if P_PLATFORM_WINDOWS
    // Include Windows before protable file dialogs
	#include <windows.h>
#endif
#include "Pipe/Extern/portable-file-dialogs.h"
#include "Pipe/Files/Paths.h"

#include <shared_mutex>


namespace p
{
#pragma region FileDialogs
	std::vector<String> ParseFilters(const TArray<DialogFileFilter>& filters)
	{
		std::vector<String> rawFilters;
		rawFilters.reserve(sizet(filters.Size()) * 2);
		for (const DialogFileFilter& filter : filters)
		{
			rawFilters.emplace_back(filter.first);
			rawFilters.emplace_back(filter.second);
		}
		return p::Move(rawFilters);
	}

	String SelectFileDialog(StringView title, StringView defaultPath,
	    const TArray<DialogFileFilter>& filters, bool alwaysShowDefaultPath)
	{
		pfd::opt options{};
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::open_file dialog(String{title}, String{defaultPath}, ParseFilters(filters), options);

		std::vector<String> files = dialog.result();
		if (files.size() > 0)
		{
			return String{files[0]};
		}
		return {};
	}

	void SelectFilesDialog(StringView title, StringView defaultPath, TArray<String>& outFiles,
	    const TArray<DialogFileFilter>& filters, bool alwaysShowDefaultPath)
	{
		pfd::opt options = pfd::opt::multiselect;
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::open_file dialog(String{title}, String{defaultPath}, ParseFilters(filters), options);

		std::vector<String> files = dialog.result();
		outFiles.Resize(i32(files.size()));
		for (u32 i = 0; i < files.size(); ++i)
		{
			outFiles[i] = Move(String{files[0]});
		}
	}

	String SelectFolderDialog(StringView title, StringView defaultPath, bool alwaysShowDefaultPath)
	{
		pfd::opt options{};
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::select_folder dialog{String{title}, String{defaultPath}, options};
		return String{dialog.result()};
	}

	String SaveFileDialog(StringView title, StringView defaultPath,
	    const TArray<DialogFileFilter>& filters, bool alwaysShowDefaultPath, bool confirmOverwrite)
	{
		pfd::opt options{};
		if (alwaysShowDefaultPath)
		{
			options = options | pfd::opt::force_path;
		}
		pfd::save_file dialog{String{title}, String{defaultPath}, ParseFilters(filters), options};
		String path{dialog.result()};
		p::ReplaceExtension(path, "rf");
		return path;
	}
#pragma endregion FileDialogs


#pragma region FileWatch
	enum FileWatcherError
	{
		FWE_FileNotFound    = -1,
		FWE_FileRepeated    = -2,
		FWE_FileOutOfScope  = -3,
		FWE_FileNotReadable = -4,
		FWE_FileRemote      = -5, /** Directory in remote file system ( create a generic FileWatcher
		                         instance	  to watch this directory ). */
		FWE_Unspecified = -6
	};

	using FileStatus    = std::filesystem::file_status;
	using FileStatusMap = TMap<Tag, FileStatus>;
	using MovedList     = TArray<TPair<Tag, Tag>>;

	FileStatus GetFileStatus(StringView path)
	{
		return std::filesystem::status(ToSTDPath(path));
	}

	bool IsReadable(FileStatus status)
	{
#if P_PLATFORM_WINDOWS == 0
		if (getuid() == 0)    // Is root?
		{
			return true;
		}
#endif
		return (status.permissions() & std::filesystem::perms::owner_read)
		    != std::filesystem::perms::none;
	}

	static String lastFileWatcherError;

	FileWatcherError TrackError(FileWatcherError error, StringView log)
	{
		switch (error)
		{
			case FWE_FileNotFound:
				lastFileWatcherError = Strings::Format("File not found ({})", log);
				break;
			case FWE_FileRepeated:
				lastFileWatcherError = Strings::Format("File repeated in watches ({})", log);
				break;
			case FWE_FileOutOfScope:
				lastFileWatcherError = Strings::Format("Symlink file out of scope ({})", log);
				break;
			case FWE_FileRemote:
				lastFileWatcherError = Strings::Format(
				    "File is located in a remote file system, use a generic watcher ({})", log);
				break;
			case FWE_Unspecified:
			default: lastFileWatcherError = String{log};
		}

		return error;
	}

	void SetLinkRealPath(String& path, String& parentPath)
	{
		FileStatus status = GetFileStatus(path);
		if (std::filesystem::is_symlink(status))
		{
			parentPath = GetParentPath(path);
			SetCanonical(path);
			return;
		}
		path = {};
	}


#pragma region BaseFileWatcher
	class BaseWatch
	{
	public:
		BaseWatch() {}

		BaseWatch(FileWatchId id, StringView path, FileWatchCallback callback, bool recursive)
		    : id(id), path(path), callback(Move(callback)), recursive(recursive)
		{}

		virtual ~BaseWatch() {}

		virtual void Watch() = 0;

		FileWatchId id = 0;
		String path;
		FileWatchCallback callback;
		bool recursive = false;
		String oldFileName;
	};

	class BaseFileWatcher
	{
	public:
		FileWatcher* fileWatcher = nullptr;
		bool initialized         = false;
		bool isGeneric           = false;


		BaseFileWatcher(FileWatcher* parent);

		virtual ~BaseFileWatcher() {}

		// Add a directory watch. On error returns FileWatchId with Error type.
		virtual FileWatchId AddWatch(
		    StringView path, FileWatchCallback callback, bool recursive) = 0;

		// Remove a directory watch. This is a brute force lazy search O(nlogn).
		virtual void RemoveWatch(StringView path) = 0;

		// Remove a directory watch. This is a map lookup O(logn).
		virtual void RemoveWatch(FileWatchId watchid) = 0;

		// Remove all directory watches
		virtual void RemoveAllWatches() = 0;

		/// Updates the watcher. Must be called often.
		virtual void Watch() = 0;

		// Handles the action
		virtual void HandleAction(BaseWatch* watch, StringView filename, FileWatchAction action,
		    StringView oldFilename = {}) = 0;

		// @return Returns a list of the directories that are being watched
		virtual TArray<String> Directories() = 0;

		// @return true if the backend init successfully
		bool IsInitialized() const
		{
			return initialized;
		}

		// @return If the link is allowed according to the current path and the state of out scope
		// links
		bool IsLinkAllowed(StringView currentPath, StringView link) const;

		/// Search if a directory already exists in the watches
		virtual bool IsPathInWatches(StringView path) const = 0;
	};

	BaseFileWatcher::BaseFileWatcher(FileWatcher* parent) : fileWatcher(parent) {}

	bool BaseFileWatcher::IsLinkAllowed(StringView currentPath, StringView link) const
	{
		return (fileWatcher->followsSymlinks && fileWatcher->allowsOutOfScopeLinks)
		    || Strings::StartsWith(currentPath, link);
	}

	class DirectorySnapshotDiff
	{
	public:
		TArray<Tag> filesDeleted;
		TArray<Tag> filesCreated;
		TArray<Tag> filesModified;
		MovedList filesMoved;
		TArray<Tag> dirsDeleted;
		TArray<Tag> dirsCreated;
		TArray<Tag> dirsModified;
		MovedList dirsMoved;
		bool dirChanged;

		void Clear();
		bool Changed();
	};


	struct DirectorySnapshot
	{
	public:
		String path;
		FileStatus pathFileState;
		FileStatusMap files;


		DirectorySnapshot() {}
		DirectorySnapshot(StringView path)
		{
			Init(path);
		}

		void Init(StringView path);
		void SetPath(StringView newPath);
		bool Exists();
		void Scan(DirectorySnapshotDiff& diff);
		FileStatusMap::Iterator NodeInFiles(FileStatus& fi);
		void AddFile(Tag filename);
		void RemoveFile(Tag filename);
		void MoveFile(Tag oldFilename, Tag filename);
		void UpdateFile(Tag filename);

	protected:
		void InitFiles();
		void DeleteAll(DirectorySnapshotDiff& diff);
	};

	void DirectorySnapshotDiff::Clear()
	{
		filesCreated.Clear();
		filesModified.Clear();
		filesMoved.Clear();
		filesDeleted.Clear();
		dirsCreated.Clear();
		dirsModified.Clear();
		dirsMoved.Clear();
		dirsDeleted.Clear();
	}

	bool DirectorySnapshotDiff::Changed()
	{
		return !filesCreated.IsEmpty() || !filesModified.IsEmpty() || !filesMoved.IsEmpty()
		    || !filesDeleted.IsEmpty() || !dirsCreated.IsEmpty() || !dirsModified.IsEmpty()
		    || !dirsMoved.IsEmpty() || !dirsDeleted.IsEmpty();
	}


	void DirectorySnapshot::Init(StringView newPath)
	{
		SetPath(newPath);
		InitFiles();
	}

	void DirectorySnapshot::SetPath(StringView newPath)
	{
		path          = String{newPath};
		pathFileState = GetFileStatus(path);
	}

	bool DirectorySnapshot::Exists()
	{
		return std::filesystem::exists(pathFileState);
	}

	void DirectorySnapshot::DeleteAll(DirectorySnapshotDiff& diff)
	{
		for (const auto& value : files)
		{
			if (std::filesystem::is_directory(value.second))
			{
				diff.dirsDeleted.Add(value.first);
			}
			else
			{
				diff.filesDeleted.Add(value.first);
			}
		}
		files.Clear();
	}

	void DirectorySnapshot::InitFiles()
	{
		files.Clear();
		for (auto& it :
		    DirectoryIterator(path, std::filesystem::directory_options::follow_directory_symlink))
		{
			files.Insert(Tag{ToString(it.path().filename())}, it.status());
		}

		TArray<Tag> filesRemoved;
		/// Remove all non regular files and non directories
		for (const auto& value : files)
		{
			if (!std::filesystem::is_regular_file(value.second)
			    && !std::filesystem::is_directory(value.second))
			{
				filesRemoved.Add(value.first);
			}
		}

		for (Tag file : filesRemoved)
		{
			files.Remove(file);
		}
	}

	void DirectorySnapshot::Scan(DirectorySnapshotDiff& diff)
	{
		diff.Clear();

		FileStatus currentFileState = GetFileStatus(path);

		diff.dirChanged = pathFileState != currentFileState;

		if (diff.dirChanged)
		{
			pathFileState = currentFileState;
		}

		/// If the directory was erased, create the events for files and directories deletion
		if (!std::filesystem::exists(currentFileState))
		{
			DeleteAll(diff);
			return;
		}

		FileStatusMap currentFiles;
		for (auto& it :
		    DirectoryIterator(path, std::filesystem::directory_options::follow_directory_symlink))
		{
			currentFiles.Insert(Tag{ToString(it.path().filename())}, it.status());
		}

		if (currentFiles.IsEmpty() && files.IsEmpty())
		{
			return;
		}

		FileStatusMap filesCpy;
		FileStatusMap::Iterator fiIt;

		if (diff.dirChanged)
		{
			filesCpy = files;
		}

		for (const auto& value : currentFiles)
		{
			const Tag& filename = value.first;
			FileStatus fi       = value.second;

			// File existed before?
			fiIt = files.FindIt(value.first);

			if (fiIt != files.end())
			{
				// Erase from the file list copy
				filesCpy.Remove(value.first);

				// File changed?
				if (fiIt->second != fi)
				{
					// Update the new file info
					files[value.first] = fi;

					// handle modified event
					if (std::filesystem::is_directory(fi))
					{
						diff.dirsModified.Add(filename);
					}
					else
					{
						diff.filesModified.Add(filename);
					}
				}
			}
			// Only add regular files or directories
			else if (std::filesystem::is_regular_file(fi) || std::filesystem::is_directory(fi))
			{
				// New file found
				files[filename] = fi;

				FileStatusMap::Iterator fit;
				Tag oldFilename;

				// Check if the same inode already existed
				if ((fit = NodeInFiles(fi)) != files.end())
				{
					oldFilename = fit->first;

					// Avoid firing a Delete event
					filesCpy.Remove(fit->first);

					// Delete the old file name
					files.Remove(fit->first);

					if (std::filesystem::is_directory(fi))
					{
						diff.dirsMoved.Add({oldFilename, filename});
					}
					else
					{
						diff.filesMoved.Add({oldFilename, filename});
					}
				}
				else
				{
					if (std::filesystem::is_directory(fi))
					{
						diff.dirsCreated.Add(filename);
					}
					else
					{
						diff.filesCreated.Add(filename);
					}
				}
			}
		}

		if (!diff.dirChanged)
		{
			return;
		}

		/// The files or directories that remains were deleted
		for (const auto& value : filesCpy)
		{
			const auto& filename = value.first;
			FileStatus status    = value.second;

			if (std::filesystem::is_directory(status))
			{
				diff.dirsDeleted.Add(filename);
			}
			else
			{
				diff.filesDeleted.Add(filename);
			}

			/// Remove the file or directory from the list of files
			files.Remove(filename);
		}

		return;
	}

	FileStatusMap::Iterator DirectorySnapshot::NodeInFiles(FileStatus& fi)
	{
		// TODO: Implement inode support
		// #if P_PLATFORM_WINDOWS == 0    // Is INode supported?
		// 		FileStatusMap::Iterator it;
		// 		for (it = files.begin(); it != files.end(); it++)
		// 		{
		// 			if (it->second.Inode == Other.Inode && it->second.filePath != fi.filePath)
		// 			{
		// 				return it;
		// 			}
		// 		}
		// #endif
		return files.end();
	}

	void DirectorySnapshot::AddFile(Tag filename)
	{
		files.Insert(Tag{GetFilename(path)}, GetFileStatus(path));
	}

	void DirectorySnapshot::RemoveFile(Tag filename)
	{
		files.Remove(filename);
	}

	void DirectorySnapshot::MoveFile(Tag oldFilename, Tag filename)
	{
		RemoveFile(oldFilename);
		AddFile(filename);
	}

	void DirectorySnapshot::UpdateFile(Tag filename)
	{
		AddFile(filename);
	}

#pragma endregion BaseFileWatcher

#pragma region GenericFileWatcher
	class GenericDirWatch;
	class GenericFileWatcher;

	class GenericWatch : public BaseWatch
	{
	public:
		GenericFileWatcher* watcher = nullptr;
		GenericDirWatch* dirWatch   = nullptr;

		GenericWatch(FileWatchId id, StringView path, FileWatchCallback callback,
		    GenericFileWatcher* fileWatcher, bool recursive);
		~GenericWatch();
		GenericWatch(const GenericWatch& other)            = delete;
		GenericWatch& operator=(const GenericWatch& other) = delete;

		void Watch();
		void WatchDir(StringView dir);
		bool IsPathInWatches(StringView path) const;
	};

	class GenericDirWatch
	{
	public:
		GenericDirWatch* parent = nullptr;
		GenericWatch* watch     = nullptr;
		DirectorySnapshot dirSnap;
		TMap<Tag, GenericDirWatch*> directories;
		bool recursive = false;

	protected:
		bool deleted = false;


	public:
		GenericDirWatch(GenericDirWatch* parent, GenericWatch* watch, StringView path,
		    bool recursive, bool reportNewFiles = false);
		~GenericDirWatch();

		void Watch(bool reportOwnChange = false);
		void WatchDir(StringView path);
		bool IsPathInWatches(StringView path) const;
		void AddChilds(bool reportNewFiles = true);
		GenericDirWatch* FindDirWatcher(StringView path);
		GenericDirWatch* FindDirWatcherFast(StringView path);

	protected:
		GenericDirWatch* CreateDirectory(Tag dir);
		void RemoveDirectory(Tag dirName);
		void MoveDirectory(Tag oldDirName, Tag dirName);
		void ResetDirectory(Tag dirName);
		void HandleAction(StringView filename, FileWatchAction action, StringView oldFilename = {});
	};

	class GenericFileWatcher : public BaseFileWatcher
	{
	protected:
		std::thread* thread     = nullptr;
		FileWatchId lastWatchId = 0;

		// Map of FileWatchId to WatchStruct pointers
		TArray<GenericWatch*> watches;
		std::shared_mutex watchesMutex;


	public:
		GenericFileWatcher(FileWatcher* parent);
		virtual ~GenericFileWatcher();

		// Add a directory watch. On error returns FileWatchId with Error type.
		FileWatchId AddWatch(StringView path, FileWatchCallback callback, bool recursive) override;

		// Remove a directory watch. This is a brute force lazy search O(nlogn).
		void RemoveWatch(StringView path) override;

		// Remove a directory watch. This is a map lookup O(logn).
		void RemoveWatch(FileWatchId watchId) override;

		void RemoveAllWatches() override;

		// Updates the watcher. Must be called often.
		void Watch() override;

		// Handles the action
		void HandleAction(BaseWatch* watch, StringView filename, FileWatchAction action,
		    StringView oldFilename = {}) override;

		// List of the directories that are being watched
		TArray<String> Directories() override;

		bool IsPathInWatches(StringView path) const override;

	private:
		void Run();
	};


	GenericWatch::GenericWatch(FileWatchId id, StringView path, FileWatchCallback callback,
	    GenericFileWatcher* fileWatcher, bool recursive)
	    : BaseWatch(id, path, Move(callback), recursive), watcher(fileWatcher)
	{
		String pathTmp{path};
		AppendPathSeparatorIfNeeded(pathTmp);
		dirWatch = new GenericDirWatch(nullptr, this, pathTmp, recursive, false);
		dirWatch->AddChilds(false);
	}

	GenericWatch::~GenericWatch()
	{
		delete dirWatch;
	}

	void GenericWatch::Watch()
	{
		dirWatch->Watch();
	}

	void GenericWatch::WatchDir(StringView pathToWatch)
	{
		dirWatch->WatchDir(pathToWatch);
	}

	bool GenericWatch::IsPathInWatches(StringView pathToCheck) const
	{
		return dirWatch->IsPathInWatches(pathToCheck);
	}


	GenericDirWatch::GenericDirWatch(GenericDirWatch* parent, GenericWatch* watch, StringView path,
	    bool recursive, bool reportNewFiles)
	    : parent(parent), watch(watch), recursive(recursive), deleted(false)
	{
		ResetDirectory(path);

		DirectorySnapshotDiff diff;
		dirSnap.Scan(diff);

		if (reportNewFiles)
		{
			dirSnap.Scan(diff);

			if (diff.Changed())
			{
				for (auto filePath : diff.filesCreated)
				{
					HandleAction(filePath.AsString(), FileWatchAction::Add);
				}
			}
		}
	}

	GenericDirWatch::~GenericDirWatch()
	{
		/// If the directory was deleted mark the files as deleted
		if (deleted)
		{
			DirectorySnapshotDiff diff;
			dirSnap.Scan(diff);

			if (!dirSnap.Exists())
			{
				for (auto filePath : diff.filesDeleted)
				{
					HandleAction(filePath.AsString(), FileWatchAction::Delete);
				}

				for (auto filePath : diff.dirsDeleted)
				{
					HandleAction(filePath.AsString(), FileWatchAction::Delete);
				}
			}
		}

		for (const auto& value : directories)
		{
			if (deleted)
			{
				/// If the directory was deleted, mark the flag for file deletion
				value.second->deleted = true;
			}
			delete value.second;
		}
	}

	void GenericDirWatch::ResetDirectory(Tag dir)
	{
		StringView path = dir.AsString();
		String pathTmp(path);

		/// Is this a recursive watch?
		if (watch->path != path)
		{
			if (path.empty()
			    || (!IsSeparator(path.at(0)) && !IsSeparator(path.at(path.size() - 1))))
			{
				/// Get the real directory
				if (parent)
				{
					pathTmp = JoinPaths(parent->dirSnap.path, path);
				}
				else
				{
					p::Error("File Watch: Invalid parent.");
				}
			}
		}

		dirSnap.SetPath(pathTmp);
	}

	void GenericDirWatch::HandleAction(
	    StringView filename, FileWatchAction action, StringView oldFilename)
	{
		watch->callback(watch->id, dirSnap.path, filename, (FileWatchAction)action, oldFilename);
	}

	void GenericDirWatch::AddChilds(bool reportNewFiles)
	{
		if (recursive)
		{
			// Create the subdirectories watchers
			for (const auto& value : dirSnap.files)
			{
				if (!std::filesystem::is_directory(value.second) || !IsReadable(value.second))
				{
					continue;
				}

				String path = JoinPaths(dirSnap.path, value.first.AsString());
				if (!IsRemotePath(path))
				{
					// Check if the directory is a symbolic link
					String currentPath;
					String link = path;
					SetLinkRealPath(link, currentPath);

					Tag dir = value.first;
					if (!link.empty())
					{
						// Avoid adding symlinks directories if it's now enabled
						if (!watch->watcher->fileWatcher->followsSymlinks)
						{
							continue;
						}

						// If it's a symlink check if the realpath exists as a watcher, or
						// if the path is outside the current dir
						if (watch->watcher->IsPathInWatches(link) || watch->IsPathInWatches(link)
						    || !watch->watcher->IsLinkAllowed(currentPath, link))
						{
							continue;
						}
						else
						{
							path = link;
							dir  = Tag{link};
						}
					}
					else
					{
						if (watch->IsPathInWatches(path) || watch->watcher->IsPathInWatches(path))
						{
							continue;
						}
					}

					if (reportNewFiles)
					{
						HandleAction(dir.AsString(), FileWatchAction::Add);
					}

					directories[dir] =
					    new GenericDirWatch(this, watch, path, recursive, reportNewFiles);

					directories[dir]->AddChilds(reportNewFiles);
				}
			}
		}
	}

	void GenericDirWatch::Watch(bool reportOwnChange)
	{
		DirectorySnapshotDiff diff;
		dirSnap.Scan(diff);

		if (reportOwnChange && diff.dirChanged && parent)
		{
			watch->callback(watch->id, GetParentPath(dirSnap.path), GetFilename(dirSnap.path),
			    FileWatchAction::Modified, StringView{});
		}

		if (diff.Changed())
		{
			/// Files
			for (auto filePath : diff.filesCreated)
			{
				HandleAction(filePath.AsString(), FileWatchAction::Add);
			}

			for (auto filePath : diff.filesModified)
			{
				HandleAction(filePath.AsString(), FileWatchAction::Modified);
			}

			for (auto filePath : diff.filesDeleted)
			{
				HandleAction(filePath.AsString(), FileWatchAction::Delete);
			}

			for (const auto& dir : diff.filesMoved)
			{
				HandleAction(dir.second.AsString(), FileWatchAction::Moved, dir.first.AsString());
			}

			/// Directories
			for (auto dirPath : diff.dirsCreated)
			{
				CreateDirectory(dirPath);
			}

			for (auto dirPath : diff.dirsModified)
			{
				HandleAction(dirPath.AsString(), FileWatchAction::Modified);
			}

			for (auto dirPath : diff.dirsDeleted)
			{
				HandleAction(dirPath.AsString(), FileWatchAction::Delete);
				RemoveDirectory(dirPath);
			}

			for (const auto& dir : diff.dirsMoved)
			{
				HandleAction(dir.second.AsString(), FileWatchAction::Moved, dir.first.AsString());
				MoveDirectory(dir.first, dir.second);
			}
		}

		/// Process the subdirectories looking for changes
		for (const auto& value : directories)
		{
			// Just watch
			value.second->Watch();
		}
	}

	void GenericDirWatch::WatchDir(StringView dir)
	{
		GenericDirWatch* watcher = watch->watcher->fileWatcher->allowsOutOfScopeLinks
		                             ? FindDirWatcher(dir)
		                             : FindDirWatcherFast(dir);

		if (watcher)
		{
			watcher->Watch(true);
		}
	}

	GenericDirWatch* GenericDirWatch::FindDirWatcherFast(StringView path)
	{
		// remove the common base ( dir should always start with the same base as the watcher )
		P_Check(!path.empty());
		P_Check(path.size() >= dirSnap.path.size());
		P_Check(dirSnap.path == path.substr(0, dirSnap.path.size()));

		if (path.size() >= dirSnap.path.size())
		{
			path = path.substr(dirSnap.path.size() - 1);
		}

		GenericDirWatch* dirWatcher = this;
		const PathIterator dirEnd   = PathIterator::CreateEnd(path);
		for (auto dirIt = PathIterator::CreateBegin(path); dirIt != dirEnd; ++dirIt)
		{
			// search the dir level in the current watcher
			auto it = dirWatcher->directories.FindIt(Tag{*dirIt});

			if (it == dirWatcher->directories.end())
			{
				// couldn't found the folder level?
				// directory not watched
				return nullptr;
			}
			// found? continue with the next level
			dirWatcher = it->second;
		}
		return dirWatcher;
	}

	GenericDirWatch* GenericDirWatch::FindDirWatcher(StringView path)
	{
		if (dirSnap.path == path)
		{
			return this;
		}
		for (const auto& dir : directories)
		{
			if (GenericDirWatch* watcher = dir.second->FindDirWatcher(path))
			{
				return watcher;
			}
		}
		return nullptr;
	}

	GenericDirWatch* GenericDirWatch::CreateDirectory(Tag dirName)
	{
		/// Check if the directory is a symbolic link
		String dirPath = JoinPaths(dirSnap.path, dirName.AsString());

		AppendPathSeparatorIfNeeded(dirPath);

		FileStatus fi = GetFileStatus(dirPath);

		if (!std::filesystem::is_directory(fi) || !IsReadable(fi) || IsRemotePath(dirPath))
		{
			return nullptr;
		}

		String curPath;
		String link = dirPath;
		SetLinkRealPath(link, curPath);

		bool skip = false;
		if (!link.empty())
		{
			/// Avoid adding symlinks directories if it's now enabled
			if (!watch->watcher->fileWatcher->followsSymlinks)
			{
				skip = true;
			}

			/// If it's a symlink check if the realpath exists as a watcher, or
			/// if the path is outside the current dir
			if (watch->watcher->IsPathInWatches(link) || watch->IsPathInWatches(link)
			    || !watch->watcher->IsLinkAllowed(curPath, link))
			{
				skip = true;
			}
			else
			{
				dirPath = link;
			}
		}
		else
		{
			if (watch->IsPathInWatches(dirPath) || watch->watcher->IsPathInWatches(dirPath))
			{
				skip = true;
			}
		}

		if (!skip)
		{
			HandleAction(dirName.AsString(), FileWatchAction::Add);

			// Creates the new directory watcher of the subfolder and check for new files
			auto* dw = new GenericDirWatch(this, watch, dirPath, recursive);
			dw->AddChilds();
			dw->Watch();

			/// Add it to the list of directories
			directories.Insert(dirName, dw);
			return dw;
		}
		return nullptr;
	}

	void GenericDirWatch::RemoveDirectory(Tag dirName)
	{
		// Search the folder, it should exists
		auto dit = directories.FindIt(dirName);
		if (dit == directories.end())
		{
			return;
		}

		GenericDirWatch* dw = dit->second;
		/// Flag it as deleted so it fire the event for every file inside deleted
		dw->deleted = true;

		/// Delete the GenericDirWatch
		delete dw;

		/// Remove the directory from the map
		directories.Remove(dit->first);
	}

	void GenericDirWatch::MoveDirectory(Tag oldDirName, Tag dirName)
	{
		/// Directory existed?
		auto it = directories.FindIt(oldDirName);
		if (it != directories.end())
		{
			GenericDirWatch* dw = it->second;
			directories.Remove(oldDirName);
			directories[dirName] = dw;
			dw->ResetDirectory(dirName);
		}
	}

	bool GenericDirWatch::IsPathInWatches(StringView path) const
	{
		if (dirSnap.path == path)
		{
			return true;
		}

		for (auto dir : directories)
		{
			if (dir.second->IsPathInWatches(path))
			{
				return true;
			}
		}
		return false;
	}


	GenericFileWatcher::GenericFileWatcher(FileWatcher* parent) : BaseFileWatcher(parent)
	{
		initialized = true;
		isGeneric   = true;
	}

	GenericFileWatcher::~GenericFileWatcher()
	{
		initialized = false;

		if (thread)
		{
			thread->join();
			delete thread;
		}

		/// Delete the watches
		for (GenericWatch* watch : watches)
		{
			delete watch;
		}
	}

	// Add a directory watch. On error returns FileWatchId with Error type.
	FileWatchId GenericFileWatcher::AddWatch(
	    StringView path, FileWatchCallback callback, bool recursive)
	{
		String dir(path);

		AppendPathSeparatorIfNeeded(dir);

		FileStatus fi = GetFileStatus(dir);

		if (!std::filesystem::is_directory(fi))
		{
			return TrackError(FWE_FileNotFound, dir);
		}
		else if (!IsReadable(fi))
		{
			return TrackError(FWE_FileNotReadable, dir);
		}
		else if (IsPathInWatches(dir))
		{
			return TrackError(FWE_FileRepeated, dir);
		}

		String link = dir;
		String parentPath;
		SetLinkRealPath(link, parentPath);

		if (link.empty())
		{
			if (IsPathInWatches(link))
			{
				return TrackError(FWE_FileRepeated, dir);
			}
			else if (!IsLinkAllowed(parentPath, link))
			{
				return TrackError(FWE_FileOutOfScope, dir);
			}
			else
			{
				dir = link;
			}
		}

		lastWatchId++;

		GenericWatch* watch = new GenericWatch(lastWatchId, dir, Move(callback), this, recursive);
		std::unique_lock lock{watchesMutex};
		watches.Add(watch);
		return watch->id;
	}

	// Remove a directory watch. This is a brute force lazy search O(nlogn).
	void GenericFileWatcher::RemoveWatch(StringView path)
	{
		for (i32 i = 0; i < watches.Size(); ++i)
		{
			GenericWatch* watch = watches[i];
			if (watch->path == path)
			{
				std::unique_lock lock{watchesMutex};
				delete watch;
				watches.RemoveAt(i);
				return;
			}
		}
	}

	void GenericFileWatcher::RemoveWatch(FileWatchId watchId)
	{
		for (i32 i = 0; i < watches.Size(); ++i)
		{
			GenericWatch* watch = watches[i];
			if (watch->id == watchId)
			{
				std::unique_lock lock{watchesMutex};
				delete watch;
				watches.RemoveAt(i);
				return;
			}
		}
	}

	void GenericFileWatcher::RemoveAllWatches()
	{
		std::unique_lock lock{watchesMutex};
		for (GenericWatch* watch : watches)
		{
			delete watch;
		}
		watches.Clear();
	}

	void GenericFileWatcher::Watch()
	{
		if (!thread)
		{
			thread = new std::thread(&GenericFileWatcher::Run, this);
		}
	}

	// Handles the action
	void GenericFileWatcher::HandleAction(
	    BaseWatch* watch, StringView filename, FileWatchAction action, StringView oldFilename)
	{
		// Not used
	}

	// List of the directories that are being watched
	TArray<String> GenericFileWatcher::Directories()
	{
		TArray<String> dirs;

		std::unique_lock lock{watchesMutex};
		for (GenericWatch* watch : watches)
		{
			dirs.Add(watch->path);
		}

		return Move(dirs);
	}

	bool GenericFileWatcher::IsPathInWatches(StringView path) const
	{
		for (GenericWatch* watch : watches)
		{
			if (watch->path == path || watch->IsPathInWatches(path))
			{
				return true;
			}
		}
		return false;
	}

	void GenericFileWatcher::Run()
	{
		do
		{
			{
				std::shared_lock lock{watchesMutex};
				for (GenericWatch* watch : watches)
				{
					watch->Watch();
				}
			}

			if (initialized)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			}
		} while (initialized);
	}
#pragma endregion GenericFileWatcher


	// #if P_PLATFORM_WINDOWS
	//  TODO: Implement other file watchers. For now we use generic only
	//  ...
	//  using DefaultFileWatcher = Win32FileWatcher
	//  #elif P_PLATFORM_MACOS
	//   using DefaultFileWatcher = FSEventsFileWatcher;
	//  #elif P_PLATFORM_IOS || P_PLATFORM_BSD || (P_PLATFORM_LINUX && P_USE_KQUEUE)
	//  ...
	//   using DefaultFileWatcher = KqueueFileWatcher;
	//  #elif P_PLATFORM_LINUX
	//  ...
	//   using DefaultFileWatcher = InotifyFileWatcher;
	//  #else
	using DefaultFileWatcher = GenericFileWatcher;
	// #endif


	FileWatcher::FileWatcher(bool useGeneric)
	{
		if (useGeneric)
		{
			fileWatcher = MakeOwned<GenericFileWatcher>(this);
		}
		else
		{
			fileWatcher = MakeOwned<DefaultFileWatcher>(this);
			if (!fileWatcher.GetUnsafe<BaseFileWatcher>()
			         ->IsInitialized())    // Fallback to generic file watcher
			{
				p::Warning("Initialization of OS file watcher failed. Fallback to generic.");
				fileWatcher = MakeOwned<GenericFileWatcher>(this);
			}
		}
	}
	FileWatcher::~FileWatcher()
	{
		Reset();
	}

	void FileWatcher::StartWatchingAsync()
	{
		fileWatcher.GetUnsafe<BaseFileWatcher>()->Watch();
	}

	FileWatchId FileWatcher::ListenPath(StringView path, bool recursive, FileWatchCallback callback)
	{
		return fileWatcher.GetUnsafe<BaseFileWatcher>()->AddWatch(path, Move(callback), recursive);
	}

	void FileWatcher::StopListening(FileWatchId id)
	{
		fileWatcher.GetUnsafe<BaseFileWatcher>()->RemoveWatch(id);
	}

	void FileWatcher::Reset()
	{
		fileWatcher.GetUnsafe<BaseFileWatcher>()->RemoveAllWatches();
	}
#pragma endregion FileWatch
}    // namespace p
