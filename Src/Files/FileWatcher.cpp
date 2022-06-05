// Copyright 2015-2022 Piperift - All rights reserved

#include "Files/FileWatcher.h"

#include "Core/Checks.h"


namespace pipe::files
{
	efsw::FileWatcher FileWatcher::fileWatcher{};


	void FileWatcher::Listener::handleFileAction(FileWatchId watchid, const std::string& dir,
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

	FileWatcher::~FileWatcher()
	{
		Reset();
	}

	void FileWatcher::StartAsync()
	{
		fileWatcher.watch();
	}

	FileWatchId FileWatcher::AddPath(StringView path, bool recursive)
	{
		FileWatchId id = fileWatcher.addWatch(std::string{path}, &listener, recursive);
		watches.Add(id);
		return id;
	}

	void FileWatcher::RemovePath(StringView path)
	{
		fileWatcher.removeWatch(std::string{path});
	}

	void FileWatcher::RemovePath(FileWatchId id)
	{
		fileWatcher.removeWatch(id);
		watches.Remove(id);
	}

	void FileWatcher::Reset()
	{
		for (FileWatchId id : watches)
		{
			fileWatcher.removeWatch(id);
		}
		watches.Empty();
	}

	void FileWatcher::AddExtension(StringView extension)
	{
		if (EnsureMsg(extension.size() <= 7, "A filtered extension can't be longer than 8 chars"))
		{
			allowedExtensions.AddUnique(extension);
		}
	}
}    // namespace pipe::files
