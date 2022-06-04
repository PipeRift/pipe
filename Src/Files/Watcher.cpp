// Copyright 2015-2022 Piperift - All rights reserved

#include "Core/Checks.h"
#include "Files/FileWatcher.h"



namespace Pipe::Files
{
	efsw::FileWatcher Watcher::fileWatcher{};


	void Watcher::Listener::handleFileAction(WatchId watchid, const std::string& dir,
	    const std::string& filename, efsw::Action action, std::string oldFilename)
	{
		// NOTE: Just for testing
		if (self.allowedExtensions.Size())
		{
			bool validExtension = false;
			for (const auto& extension : self.allowedExtensions)
			{
				validExtension |= Strings::EndsWith(filename, extension);
			}
			if (!validExtension)
			{
				return;
			}
		}

		switch (action)
		{
			case efsw::Actions::Add:
				std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Added"
				          << std::endl;
				break;
			case efsw::Actions::Delete:
				std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Delete"
				          << std::endl;
				break;
			case efsw::Actions::Modified:
				std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Modified"
				          << std::endl;
				break;
			case efsw::Actions::Moved:
				std::cout << "DIR (" << dir << ") FILE (" << filename << ") has event Moved from ("
				          << oldFilename << ")" << std::endl;
				break;
			default: std::cout << "Should never happen!" << std::endl;
		}
	}

	Watcher::~Watcher()
	{
		Reset();
	}

	void Watcher::StartAsync()
	{
		fileWatcher.watch();
	}

	WatchId Watcher::AddPath(StringView path, bool recursive)
	{
		WatchId id = fileWatcher.addWatch(std::string{path}, &listener, recursive);
		watches.Add(id);
		return id;
	}

	void Watcher::RemovePath(StringView path)
	{
		fileWatcher.removeWatch(std::string{path});
	}

	void Watcher::RemovePath(WatchId id)
	{
		fileWatcher.removeWatch(id);
		watches.Remove(id);
	}

	void Watcher::Reset()
	{
		for (WatchId id : watches)
		{
			fileWatcher.removeWatch(id);
		}
		watches.Empty();
	}

	void Watcher::AddExtension(StringView extension)
	{
		if (EnsureMsg(extension.size() <= 7, "A filtered extension can't be longer than 8 chars"))
		{
			allowedExtensions.AddUnique(extension);
		}
	}
}    // namespace Pipe::Files
