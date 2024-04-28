// Copyright 2015-2024 Piperift - All rights reserved

#include "PipeFiles.h"

#include "Pipe/Core/Checks.h"

#include <portable-file-dialogs.h>

#include <efsw/efsw.hpp>


namespace p
{
#pragma region FileWatch
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
		void handleFileAction(FileListenerId watchid, const std::string& dir,
		    const std::string& filename, efsw::Action action, std::string oldFilename) override;
	};


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

	FileWatcher::FileWatcher() : fileWatcher{MakeOwned<efsw::FileWatcher>()} {}
	FileWatcher::~FileWatcher()
	{
		Reset();
	}

	void FileWatcher::StartAsync()
	{
		fileWatcher.Get<efsw::FileWatcher>()->watch();
	}

	FileListenerId FileWatcher::ListenPath(
	    StringView path, bool recursive, FileWatchCallback callback)
	{
		auto listener          = MakeOwned<FileWatchListener>(Move(callback));
		FileListenerId watchId = fileWatcher.Get<efsw::FileWatcher>()->addWatch(
		    std::string{path}, listener.Get(), recursive);
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
