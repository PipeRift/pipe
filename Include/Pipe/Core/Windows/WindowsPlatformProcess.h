// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Generic/GenericPlatformProcess.h"
#include "Pipe/Export.h"


namespace p::core
{
	struct PIPE_API WindowsPlatformProcess : public GenericPlatformProcess
	{
		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static void ShowFolder(StringView path);

		static bool CreatePipe(void*& readPipe, void*& writePipe, bool writePipeLocal = false);
		static void ClosePipe(void* readPipe, void* writePipe);
		static String ReadPipe(void* readPipe)                   = delete;
		static bool ReadPipe(void* readPipe, TArray<u8>& output) = delete;
		static bool WritePipe(
		    void* writePipe, const String& msg, String* outWritten = nullptr) = delete;
		static bool WritePipe(
		    void* writePipe, TSpan<const u8> data, i32* outWrittenLength = nullptr) = delete;
	};

	using PlatformProcess = WindowsPlatformProcess;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
