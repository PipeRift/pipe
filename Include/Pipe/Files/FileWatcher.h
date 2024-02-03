// Copyright 2015-2023 Piperift - All rights reserved

#pragma once

#include "Pipe/Core/FixedString.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Files/Paths.h"

#include <efsw/efsw.hpp>


namespace p
{
	enum class FileWatchAction
	{
		Add      = efsw::Actions::Add,
		Delete   = efsw::Actions::Delete,
		Modified = efsw::Actions::Modified,
		Moved    = efsw::Actions::Moved
	};

	using FileListenerId    = efsw::WatchID;
	using FileWatchCallback = std::function<void(
	    StringView path, StringView filename, FileWatchAction action, StringView oldFilename)>;


	struct PIPE_API FileWatchListener : public efsw::FileWatchListener
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


	struct PIPE_API FileWatcher
	{
		efsw::FileWatcher fileWatcher;
		p::TArray<TOwnPtr<FileWatchListener>> listeners;

	public:
		FileWatcher() {}
		~FileWatcher();
		FileWatcher(const FileWatcher& other)            = delete;
		FileWatcher& operator=(const FileWatcher& other) = delete;

		FileListenerId ListenPath(StringView path, bool recursive, FileWatchCallback callback);
		void StopListening(FileListenerId id);
		void Reset();

		void StartAsync();
	};
}    // namespace p
