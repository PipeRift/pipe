// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Core/FixedString.h"
#include "Files/Paths.h"

#include <efsw/efsw.hpp>


namespace p::files
{
	using FileWatchId = efsw::WatchID;

	struct FileWatcher
	{
	protected:
		class Listener : public efsw::FileWatchListener
		{
			FileWatcher& self;

		public:
			Listener(FileWatcher& self) : self{self} {}

			PIPE_API void handleFileAction(FileWatchId watchid, const std::string& dir,
			    const std::string& filename, efsw::Action action, std::string oldFilename) override;
		};
		friend Listener;

		static efsw::FileWatcher fileWatcher;

		TArray<FixedString<7>> allowedExtensions;
		TArray<FileWatchId> watches;
		Listener listener;


	public:
		PIPE_API FileWatcher() : listener{*this} {}
		PIPE_API ~FileWatcher();

		// Adds one extension. If there are no extensions, all are allowed
		PIPE_API FileWatchId AddPath(StringView path, bool recursive = true);
		PIPE_API void RemovePath(StringView path);
		PIPE_API void RemovePath(FileWatchId id);
		PIPE_API void Reset();
		PIPE_API void AddExtension(StringView extension);

		static PIPE_API void StartAsync();
	};
}    // namespace p::files

namespace p
{
	using namespace p::files;
}
