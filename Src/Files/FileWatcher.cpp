// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Files/FileWatcher.h"

#include "Pipe/Core/Checks.h"


namespace p
{
	void FileWatchListener::handleFileAction(FileListenerId watchid, const std::string& dir,
	    const std::string& filename, efsw::Action action, std::string oldFilename)
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

	FileWatcher::~FileWatcher()
	{
		Reset();
	}


	void FileWatcher::StartAsync()
	{
		fileWatcher.watch();
	}

	FileListenerId FileWatcher::ListenPath(
	    StringView path, bool recursive, FileWatchCallback callback)
	{
		auto listener          = MakeOwned<FileWatchListener>(Move(callback));
		FileListenerId watchId = fileWatcher.addWatch(std::string{path}, listener.Get(), recursive);
		listener->watchId      = watchId;
		listeners.Add(Move(listener));
		return watchId;
	}

	void FileWatcher::StopListening(FileListenerId id)
	{
		fileWatcher.removeWatch(id);
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
				fileWatcher.removeWatch(listener->watchId);
			}
		}
		listeners.Clear();
	}
}    // namespace p
