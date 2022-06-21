// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Generic/GenericPlatformProcess.h"
#include "Pipe/Export.h"


namespace p::core
{
	struct PIPE_API MacPipeHandle : public GenericPipeHandle
	{
	protected:
		void* readPipe;
		void* writePipe;


	public:
		explicit MacPipeHandle(bool writePipeLocal = false);
		MacPipeHandle(MacPipeHandle&& other) noexcept;
		~MacPipeHandle()
		{
			Close();
		}

		bool Read(String& output);
		bool Read(TArray<u8>& output);
		bool Write(const String& msg, String* outWritten = nullptr);
		bool Write(TSpan<const u8> data, i32* outWrittenLength = nullptr);
		void Close();
		bool IsValid() const
		{
			return readPipe && writePipe;
		}
	};
	using PipeHandle = MacPipeHandle;


	struct PIPE_API MacPlatformProcess : public GenericPlatformProcess
	{
		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static void ShowFolder(StringView path);
	};
	using PlatformProcess = MacPlatformProcess;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
