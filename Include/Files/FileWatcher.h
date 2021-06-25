// Copyright 2015-2021 Piperift - All rights reserved

#pragma once

#include "Files/Paths.h"
#include "Strings/FixedString.h"

#include <efsw/efsw.hpp>


namespace Rift
{
	using FileWatchId = efsw::WatchID;

	struct CORE_API FileWatcher
	{
	protected:
		class Listener : public efsw::FileWatchListener
		{
			FileWatcher& self;

		public:
			Listener(FileWatcher& self) : self{self} {}

			void handleFileAction(FileWatchId watchid, const std::string& dir,
			    const std::string& filename, efsw::Action action, std::string oldFilename) override;
		};
		friend Listener;

		static efsw::FileWatcher fileWatcher;

		TArray<FixedString<7>> allowedExtensions;
		TArray<FileWatchId> watches;
		Listener listener;


	public:
		FileWatcher() : listener{*this} {}
		~FileWatcher();

		// Adds one extension. If there are no extensions, all are allowed
		FileWatchId AddPath(StringView path, bool recursive = true);
		void RemovePath(StringView path);
		void RemovePath(FileWatchId id);
		void Reset();
		void AddExtension(StringView extension);

		static void StartAsync();
	};
}    // namespace Rift
