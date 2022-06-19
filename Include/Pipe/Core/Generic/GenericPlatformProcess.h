// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Span.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Export.h"


namespace p::core
{
	struct PIPE_API GenericPlatformProcess
	{
		static StringView GetExecutableFile() = delete;
		static StringView GetExecutablePath() = delete;
		static StringView GetBasePath()       = delete;

		/** Content saved to compiler or project directories should be rerouted to user directories
		 * instead **/
		static bool ShouldSaveToUserDir()
		{
			return false;
		}
		static void ShowFolder(StringView path);


		static bool CreatePipe(
		    void*& readPipe, void*& writePipe, bool writePipeLocal = false) = delete;
		static void ClosePipe(void* readPipe, void* writePipe)              = delete;
		static String ReadPipe(void* readPipe)                              = delete;
		static bool ReadPipe(void* readPipe, TArray<u8>& output)            = delete;
		static bool WritePipe(
		    void* writePipe, const String& msg, String* outWritten = nullptr) = delete;
		static bool WritePipe(
		    void* writePipe, TSpan<const u8> data, i32* outWrittenLength = nullptr) = delete;
	};
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
