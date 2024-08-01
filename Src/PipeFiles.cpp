// Copyright 2015-2024 Piperift - All rights reserved

#include "PipeFiles.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Extern/portable-file-dialogs.h"

#include <efsw/efsw.hpp>


namespace p
{
#pragma region FileWatch

	enum class FileWatcherError : i8
	{
		FileNotFound    = -1,
		FileRepeated    = -2,
		FileOutOfScope  = -3,
		FileNotReadable = -4,
		FileRemote = -5, /** Directory in remote file system ( create a generic FileWatcher instance
		                    to watch this directory ). */
		Unspecified = -6
	};

	static String lastFileWatcherError;

	FileWatcherError TrackError(FileWatcherError error, StringView log)
	{
		switch (error)
		{
			case FileWatcherError::FileNotFound:
				lastFileWatcherError = Strings::Format("File not found ({})", log);
				break;
			case FileWatcherError::FileRepeated:
				lastFileWatcherError = Strings::Format("File repeated in watches ({})", log);
				break;
			case FileWatcherError::FileOutOfScope:
				lastFileWatcherError = Strings::Format("Symlink file out of scope ({})", log);
				break;
			case FileWatcherError::FileRemote:
				lastFileWatcherError = Strings::Format(
				    "File is located in a remote file system, use a generic watcher ({})", log);
				break;
			case FileWatcherError::Unspecified:
			default: lastFileWatcherError = String{log};
		}

		return error;
	}

#pragma region BaseFileWatcher
	using FileWatchId = i32;

	class BaseWatcher
	{
	public:
		BaseWatcher();

		BaseWatcher(FileWatchId id, String directory, FileWatchListener* listener, bool recursive)
		    : id(id), directory(directory), listener(listener), recursive(recursive)
		{}

		virtual ~BaseWatcher() {}

		virtual void Watch() = 0;

		FileWatchId id = 0;
		String directory;
		FileWatchListener* listener = nullptr;
		bool recursive              = false;
		String oldFileName;
	};

	class BaseFileWatcher
	{
	public:
		BaseFileWatcher(FileWatcher* parent);

		virtual ~BaseFileWatcher();

		// Add a directory watch. On error returns FileWatchId with Error type.
		virtual FileWatchId AddWatch(
		    const String& directory, FileWatchListener* watcher, bool recursive) = 0;

		// Remove a directory watch. This is a brute force lazy search O(nlogn).
		virtual void RemoveWatch(const String& directory) = 0;

		// Remove a directory watch. This is a map lookup O(logn).
		virtual void RemoveWatch(FileWatchId watchid) = 0;

		/// Updates the watcher. Must be called often.
		virtual void Watch() = 0;

		// Handles the action
		virtual void HandleAction(BaseWatcher* watch, const String& filename, unsigned long action,
		    String oldFilename = "") = 0;

		// @return Returns a list of the directories that are being watched
		virtual std::list<String> Directories() = 0;

		// @return true if the backend init successfully
		bool IsInitialized() const
		{
			return initialized;
		}

		// @return If the link is allowed according to the current path and the state of out scope
		// links
		bool IsLinkAllowed(const String& currentPath, const String& link);

		/// Search if a directory already exists in the watches
		virtual bool IsPathInWatches(const String& path) = 0;

		FileWatcher* fileWatcher = nullptr;
		bool initialized         = false;
		bool isGeneric           = false;
	};

	BaseFileWatcher::BaseFileWatcher(FileWatcher* parent) : fileWatcher(parent) {}

	bool BaseFileWatcher::IsLinkAllowed(const String& currentPath, const String& link)
	{
		return (fileWatcher->followsSymlinks && fileWatcher->allowsOutOfScopeLinks)
		    || Strings::StartsWith(currentPath, link);
	}
#pragma endregion BaseFileWatcher

#pragma region GenericFileWatcher
	class GenericDirWatcher;

	class WatcherGeneric : public BaseWatcher
	{
	public:
		BaseFileWatcher* watcherImpl;
		GenericDirWatcher* dirWatch;

		WatcherGeneric(FileWatchId id, const String& directory, FileWatchListener* fwl,
		    BaseFileWatcher* fw, bool recursive);

		~WatcherGeneric();

		void Watch();

		void WatchDir(String dir);

		bool IsPathInWatches(String path);
	};

	class GenericDirWatcher
	{
	public:
		using DirWatchMap = std::map<String, GenericDirWatcher*>;

		GenericDirWatcher* parent = nullptr;
		WatcherGeneric* watch;
		DirectorySnapshot dirSnap;
		DirWatchMap directories;
		bool recursive = false;

		GenericDirWatcher(GenericDirWatcher* parent, WatcherGeneric* ws, const String& directory,
		    bool recursive, bool reportNewFiles = false);

		~GenericDirWatcher();

		void Watch(bool reportOwnChange = false);

		void WatchDir(String& dir);

		static bool IsDir(const String& directory);

		bool IsPathInWatches(String path);

		void AddChilds(bool reportNewFiles = true);

		GenericDirWatcher* FindDirWatcher(String dir);

		GenericDirWatcher* FindDirWatcherFast(String dir);

	protected:
		bool deleted = false;

		GenericDirWatcher* createDirectory(String newdir);

		void removeDirectory(String dir);

		void moveDirectory(String oldDir, String newDir);

		void resetDirectory(String directory);

		void handleAction(const String& filename, unsigned long action, String oldFilename = "");
	};


	class GenericFileWatcher : public BaseFileWatcher
	{
	public:
		typedef std::list<WatcherGeneric*> WatchList;

	protected:
		std::thread* thread     = nullptr;
		FileWatchId lastWatchId = 0;

		// Map of FileWatchId to WatchStruct pointers
		WatchList watches;

		Mutex watchesLock;


	public:
		GenericFileWatcher(FileWatcher* parent);

		virtual ~GenericFileWatcher();

		// Add a directory watch. On error returns FileWatchId with Error type.
		FileWatchId AddWatch(const String& directory, FileWatchListener* watcher, bool recursive);

		// Remove a directory watch. This is a brute force lazy search O(nlogn).
		void RemoveWatch(const String& directory);

		// Remove a directory watch. This is a map lookup O(logn).
		void RemoveWatch(FileWatchId watchid, WatchList::iterator it = watches.begin());

		// Updates the watcher. Must be called often.
		void Watch();

		// Handles the action
		void HandleAction(
		    Watcher* watch, const String& filename, unsigned long action, String oldFilename = "");

		// List of the directories that are being watched
		std::list<String> Directories();

	protected:
		bool IsPathInWatches(const String& path);

	private:
		void Run();
	};

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
		WatchList::iterator it = watches.begin();

		for (; it != watches.end(); ++it)
		{
			delete (*it);
		}
	}

	// Add a directory watch. On error returns FileWatchId with Error type.
	FileWatchId GenericFileWatcher::AddWatch(
	    const String& directory, FileWatchListener* watcher, bool recursive)
	{
		String dir(directory);

		FileSystem::dirAddSlashAtEnd(dir);

		FileInfo fi(dir);

		if (!fi.isDirectory())
		{
			return TrackError(FileWatcherError::FileNotFound, dir);
		}
		else if (!fi.isReadable())
		{
			return TrackError(FileWatcherError::FileNotReadable, dir);
		}
		else if (IsPathInWatches(dir))
		{
			return TrackError(FileWatcherError::FileRepeated, dir);
		}

		String curPath;
		String link(FileSystem::getLinkRealPath(dir, curPath));

		if ("" != link)
		{
			if (IsPathInWatches(link))
			{
				return TrackError(FileWatcherError::FileRepeated, dir);
			}
			else if (!IsLinkAllowed(curPath, link))
			{
				return TrackError(FileWatcherError::FileOutOfScope, dir);
			}
			else
			{
				dir = link;
			}
		}

		lastWatchId++;

		WatcherGeneric* pWatch = new WatcherGeneric(lastWatchId, dir, watcher, this, recursive);

		Lock lock(watchesLock);
		watches.push_back(pWatch);

		return pWatch->id;
	}

	// Remove a directory watch. This is a brute force lazy search O(nlogn).
	void GenericFileWatcher::RemoveWatch(const String& directory)
	{
		WatchList::iterator it = watches.begin();

		for (; it != watches.end(); ++it)
		{
			if ((*it)->Directory == directory)
			{
				WatcherGeneric* watch = (*it);

				Lock lock(watchesLock);

				watches.erase(it);

				efSAFE_DELETE(watch);

				return;
			}
		}
	}

	void GenericFileWatcher::RemoveWatch(
	    FileWatchId watchid) WatchList::iterator it = watches.begin()
	{
		for (; it != watches.end(); ++it)
		{
			if ((*it)->id == watchid)
			{
				WatcherGeneric* watch = (*it);

				Lock lock(watchesLock);

				watches.erase(it);

				efSAFE_DELETE(watch);

				return;
			}
		}
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
	    Watcher* watch, const String& filename, unsigned long action, String oldFilename = "")
	{
		// Not used
	}

	// List of the directories that are being watched
	std::list<String> GenericFileWatcher::Directories()
	{
		std::list<String> dirs;

		Lock lock(watchesLock);

		WatchList::iterator it = watches.begin();

		for (; it != watches.end(); ++it)
		{
			dirs.push_back((*it)->Directory);
		}

		return Move(dirs);
	}

	bool GenericFileWatcher::IsPathInWatches(const String& path)
	{
		WatchList::iterator it = watches.begin();

		for (; it != watches.end(); ++it)
		{
			if ((*it)->Directory == path || (*it)->IsPathInWatches(path))
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
				Lock lock(watchesLock);

				WatchList::iterator it = watches.begin();

				for (; it != watches.end(); ++it)
				{
					(*it)->Watch();
				}
			}

			if (initialized)
			{
				System::sleep(1000);
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


	static_assert(p::u8(FileWatchAction::Add) == efsw::Actions::Add);
	static_assert(p::u8(FileWatchAction::Delete) == efsw::Actions::Delete);
	static_assert(p::u8(FileWatchAction::Modified) == efsw::Actions::Modified);
	static_assert(p::u8(FileWatchAction::Moved) == efsw::Actions::Moved);

	struct FileWatchListener : public efsw::FileWatchListener
	{
		friend struct FileWatcher;

	private:
		FileListenerId watchId;
		FileWatchCallback callback;

	public:
		FileWatchListener(FileWatchCallback callback) : callback{Move(callback)} {}

		bool operator==(FileListenerId other) const
		{
			return watchId == other;
		}

	private:
		void HandleFileAction(FileListenerId watchid, const String& dir, const String& filename,
		    efsw::Action action, String oldFilename) override;
	};


	void FileWatchListener::HandleFileAction(FileListenerId watchid, const String& dir,
	    const String& filename, efsw::Action action, String oldFilename)
	{
		/*if (allowedExtensions.Size())
		{
		    bool validExtension = false;
		    for (const auto& extension : allowedExtensions)
		    {
		        validExtension |= Strings::EndsWith(filename, extension);
		    }
		    if (!validExtension)
		    {
		        return;
		    }
		}*/
		callback(dir, filename, FileWatchAction(action), oldFilename);
	}

	FileWatcher::FileWatcher(bool useGeneric)
	{
		if (useGeneric)
		{
			fileWatcher = MakeOwned<GenericFileWatcher>(this);
		}
		else
		{
			fileWatcher = MakeOwned<DefaultFileWatcher>(this);
			if (!fileWatcher->IsInitialized())    // Fallback to generic file watcher
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
		fileWatcher.Get<BaseFileWatcher>()->Watch();
	}

	FileListenerId FileWatcher::ListenPath(
	    StringView path, bool recursive, FileWatchCallback callback)
	{
		auto listener = MakeOwned<FileWatchListener>(Move(callback));
		FileListenerId watchId =
		    fileWatcher.Get<efsw::FileWatcher>()->addWatch(String{path}, listener.Get(), recursive);
		listener->watchId = watchId;
		listeners.Add(Move(listener));
		return watchId;
	}

	void FileWatcher::StopListening(FileListenerId id)
	{
		fileWatcher.Get<efsw::FileWatcher>()->removeWatch(id);
		listeners.RemoveIfSwap([id](const auto& listener) {
			return !listener || listener->watchId == id;
		});
	}

	void FileWatcher::Reset()
	{
		for (const auto& listener : listeners)
		{
			if (listener)
			{
				fileWatcher.Get<efsw::FileWatcher>()->removeWatch(listener->watchId);
			}
		}
		listeners.Clear();
	}
#pragma endregion FileWatch


#pragma region FileDialogs
	std::vector<std::string> ParseFilters(const TArray<DialogFileFilter>& filters)
	{
		std::vector<std::string> rawFilters;
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
		pfd::open_file dialog(
		    std::string{title}, std::string{defaultPath}, ParseFilters(filters), options);

		std::vector<std::string> files = dialog.result();
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
		pfd::open_file dialog(
		    std::string{title}, std::string{defaultPath}, ParseFilters(filters), options);

		std::vector<std::string> files = dialog.result();
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
		pfd::select_folder dialog{std::string{title}, std::string{defaultPath}, options};
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
		pfd::save_file dialog{
		    std::string{title}, std::string{defaultPath}, ParseFilters(filters), options};
		String path{dialog.result()};
		p::ReplaceExtension(path, "rf");
		return path;
	}
#pragma endregion FileDialogs
}    // namespace p
