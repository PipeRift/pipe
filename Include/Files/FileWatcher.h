// Copyright 2015-2022 Piperift - All rights reserved

#pragma once

#include "Core/FixedString.h"
#include "Files/Paths.h"

#include <efsw/efsw.hpp>


namespace Pipe::Files
{
	using WatchId = efsw::WatchID;

	struct Watcher
	{
	protected:
		class Listener : public efsw::FileWatchListener
		{
			Watcher& self;

		public:
			Listener(Watcher& self) : self{self} {}

			CORE_API void handleFileAction(WatchId watchid, const std::string& dir,
			    const std::string& filename, efsw::Action action, std::string oldFilename) override;
		};
		friend Listener;

		static efsw::FileWatcher fileWatcher;

		TArray<FixedString<7>> allowedExtensions;
		TArray<WatchId> watches;
		Listener listener;


	public:
		CORE_API Watcher() : listener{*this} {}
		CORE_API ~Watcher();

		// Adds one extension. If there are no extensions, all are allowed
		CORE_API WatchId AddPath(StringView path, bool recursive = true);
		CORE_API void RemovePath(StringView path);
		CORE_API void RemovePath(WatchId id);
		CORE_API void Reset();
		CORE_API void AddExtension(StringView extension);

		static CORE_API void StartAsync();
	};
}    // namespace Pipe::Files

namespace Pipe
{
	using namespace Pipe::Files;
}
