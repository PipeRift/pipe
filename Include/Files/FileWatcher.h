// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Files/Paths.h"
#include "Strings/FixedString.h"

#include <efsw/efsw.hpp>


namespace Rift
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

			CORE_API void handleFileAction(FileWatchId watchid, const std::string& dir,
			    const std::string& filename, efsw::Action action, std::string oldFilename) override;
		};
		friend Listener;

		static efsw::FileWatcher fileWatcher;

		TArray<FixedString<7>> allowedExtensions;
		TArray<FileWatchId> watches;
		Listener listener;


	public:
		CORE_API FileWatcher() : listener{*this} {}
		CORE_API ~FileWatcher();

		// Adds one extension. If there are no extensions, all are allowed
		CORE_API FileWatchId AddPath(StringView path, bool recursive = true);
		CORE_API void RemovePath(StringView path);
		CORE_API void RemovePath(FileWatchId id);
		CORE_API void Reset();
		CORE_API void AddExtension(StringView extension);

		static CORE_API void StartAsync();
	};
}    // namespace Rift
