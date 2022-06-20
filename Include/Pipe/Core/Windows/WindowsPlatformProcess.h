// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Generic/GenericPlatformProcess.h"
#include "Pipe/Export.h"


namespace p::core
{
	struct PIPE_API WindowsPipeHandle : public GenericPipeHandle
	{
		void* readPipe  = nullptr;
		void* writePipe = nullptr;


		explicit WindowsPipeHandle(bool writePipeLocal = false);
		WindowsPipeHandle(WindowsPipeHandle&& other) noexcept;
		~WindowsPipeHandle()
		{
			Close();
		}

		bool Read(String& output);
		bool Read(TArray<u8>& output);
		bool Write(const String& msg, String* outWritten = nullptr);
		bool Write(TSpan<const u8> data, i32* outWrittenLength = nullptr);
		void Close();
	};
	using PipeHandle = WindowsPipeHandle;


	struct PIPE_API WindowsPlatformProcess : public GenericPlatformProcess
	{
		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static void ShowFolder(StringView path);
	};
	using PlatformProcess = WindowsPlatformProcess;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
