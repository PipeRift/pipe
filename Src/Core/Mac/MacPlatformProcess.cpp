// Copyright 2015-2022 Piperift - All rights reserved

#if PLATFORM_MACOS
#	include "Pipe/Core/Mac/MacPlatformProcess.h"
#	include "Pipe/Core/PlatformMisc.h"
#	include "Pipe/Core/String.h"
#	include "Pipe/Core/FixedString.h"
#	include "Pipe/Files/Paths.h"
#	include "Pipe/Files/Files.h"

#	include <mach-o/dyld.h>
#	include <mach/thread_act.h>
#	include <mach/thread_policy.h>
#	include <libproc.h>


namespace p::core
{
	MacPipeHandle::MacPipeHandle(bool writePipeLocal)
	{
		int pipefd[2];
		pipe(pipefd);

		fcntl(pipefd[0], F_SETFL, O_NONBLOCK);
		fcntl(pipefd[1], F_SETFL, O_NONBLOCK);

		// create an NSFileHandle from the descriptor
		readPipe  = [[NSFileHandle alloc] initWithFileDescriptor:pipefd[0]];
		writePipe = [[NSFileHandle alloc] initWithFileDescriptor:pipefd[1]];
	}

	MacPipeHandle::MacPipeHandle(MacPipeHandle&& other) noexcept
	    : readPipe{other.readPipe}, writePipe{other.writePipe}
	{
		other.readPipe  = nullptr;
		other.writePipe = nullptr;
	}

	void MacPipeHandle::Close()
	{
		if (readPipe)
		{
			close([(NSFileHandle*)readPipe fileDescriptor]);
			[(NSFileHandle*)readPipe release];
			readPipe = nullptr;
		}
		if (writePipe)
		{
			close([(NSFileHandle*)writePipe fileDescriptor]);
			[(NSFileHandle*)writePipe release];
			writePipe = nullptr;
		}
	}

	bool MacPipeHandle::Read(String& output)
	{
		if (!readPipe)
		{
			return false;
		}

		const i32 initialSize = output.size();

		static constexpr i32 bufferSize = 8192;
		AnsiChar buffer[bufferSize];
		i32 bytesRead = 0;
		do
		{
			bytesRead = read([(NSFileHandle*)readPipe fileDescriptor], buffer, (bufferSize - 1));
			if (bytesRead > 0)
			{
				Strings::ConvertTo<String>(TStringView<AnsiChar>{buffer, bytesRead}, output);
			}
		} while (bytesRead > 0);

		return output.size() > initialSize;
	}

	bool MacPipeHandle::Read(TArray<u8>& output)
	{
		if (!readPipe)
		{
			return false;
		}

		static constexpr i32 bufferSize = 32768;
		const i32 initialSize           = output.Size();
		output.Resize(initialSize + i32(bytesAvailable));

		i32 bytesRead =
		    read([(NSFileHandle*)readPipe fileDescriptor], output.Data() + initialSize, bufferSize);
		if (bytesRead > 0)
		{
			if (bytesRead < bufferSize)
			{
				output.Resize(initialSize + i32(bytesRead));
			}
			return true;
		}
		return false;
	}

	bool MacPipeHandle::Write(const String& msg, String* outWritten)
	{
		if (msg.empty() || !writePipe)
		{
			return false;
		}

		const u32 bytesAvailable = msg.size();
		auto* buffer             = new Char8[bytesAvailable + 2];
		for (uint32 i = 0; i < bytesAvailable; ++i)
		{
			buffer[i] = static_cast<Char8>(msg[i]);
		}
		buffer[bytesAvailable] = static_cast<Char8>('\n');

		// Write to pipe
		u32 bytesWritten =
		    write([(NSFileHandle*)writePipe fileDescriptor], buffer, bytesAvailable + 1);

		// Get written message
		if (outWritten)
		{
			Strings::ConvertTo<String>(TStringView<Char8>{buffer, bytesWritten}, *outWritten);
		}

		delete[] buffer;
		return (bytesWritten == bytesAvailable);
	}

	bool MacPipeHandle::Write(TSpan<const u8> data, i32* outWrittenLength)
	{
		if (data.IsEmpty() || !writePipe)
		{
			return false;
		}

		const u32 bytesWritten =
		    write([(NSFileHandle*)writePipe fileDescriptor], data.Data(), data.Size());

		// Get written Data Length
		if (outDataLength)
		{
			*outDataLength = static_cast<i32>(bytesWritten);
		}

		return (bytesWritten == data.Size());
	}

	StringView MacPlatformProcess::GetExecutableFile()
	{
		static String filePath;
		if (filePath.empty())
		{
			TFixedString<PlatformMisc::GetMaxPathLength(), char> rawPath{};
			u32 size{rawPath.size()};
			if (_NSGetExecutablePath(rawPath.data(), &size) != 0)
			{
				// Failed to retrive a path
				return {};
			}

			filePath = Strings::Convert<String>(
			    TStringView<char>{rawPath.data(), Strings::Length(rawPath.data())});
		}
		return filePath;
	}
	StringView MacPlatformProcess::GetExecutablePath()
	{
		return GetParentPath(GetExecutableFile());
	}

	StringView MacPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}

	void MacPlatformProcess::ShowFolder(StringView path)
	{
		if (!files::Exists(path))
		{
			return;
		}

		NotImplemented;
		return;
	}
}    // namespace p::core
#endif
