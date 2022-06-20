// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Generic/GenericPlatformProcess.h"
#include "Pipe/Export.h"


namespace p::core
{
	struct PIPE_API LinuxPipeHandle : public GenericPipeHandle
	{
	protected:
		i32 readPipe;
		i32 writePipe;
		bool valid = false;


	public:
		explicit LinuxPipeHandle(bool writePipeLocal = false);
		LinuxPipeHandle(LinuxPipeHandle&& other) noexcept;
		~LinuxPipeHandle()
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
			return valid;
		}
	};
	using PipeHandle = LinuxPipeHandle;


	struct PIPE_API LinuxPlatformProcess : public GenericPlatformProcess
	{
		static StringView GetExecutableFile();
		static StringView GetExecutablePath();
		static StringView GetBasePath();

		static void ShowFolder(StringView path);
	};
	using PlatformProcess = LinuxPlatformProcess;
}    // namespace p::core

namespace p
{
	using namespace p::core;
}
