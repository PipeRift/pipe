// Copyright 2015-2022 Piperift - All rights reserved

#if PLATFORM_LINUX
#	include "Pipe/Core/Linux/LinuxPlatformProcess.h"
#	include "Pipe/Core/PlatformMisc.h"
#	include "Pipe/Core/String.h"
#	include "Pipe/Core/FixedString.h"
#	include "Pipe/Files/Paths.h"
#	include "Pipe/Files/Files.h"

#	include <unistd.h>


namespace p::core
{
	LinuxPipeHandle::LinuxPipeHandle(bool writePipeLocal)
	{
		i32 pipeFd[2];
		if (-1 == pipe(PipeFd))
		{
			Log::Warning("pipe() failed with errno = {} ({})", errno,
			    Strings::Convert<String>(strerror(errno)));
			valid = false;
		}
		readPipe  = pipeFd[0];
		writePipe = pipeFd[1];
		valid = true;
	}

	LinuxPipeHandle::LinuxPipeHandle(LinuxPipeHandle&& other) noexcept
		: readPipe{other.readPipe}, writePipe{other.writePipe}, valid{other.valid}
	{
		other.valid = false;
	}

	void LinuxPipeHandle::Close()
	{
		if (valid)
		{
			close(readPipe);
			close(writePipe);
			valid = false;
		}
	}

	bool LinuxPipeHandle::Read(String& output)
	{
		if (!valid)
		{
			return false;
		}

		static constexpr i32 kBufferSize = 4096;
		AnsiChar buffer[kBufferSize];

		i32 bytesAvailable = 0;
		if (ioctl(readPipe, FIONREAD, &bytesAvailable) == 0)
		{
			if (bytesAvailable > 0)
			{
				i32 bytesRead = read(readPipe, buffer, kBufferSize - 1);
				if (bytesRead > 0)
				{
					Strings::ConvertTo<String, TChar>({buffer, bytesRead}, output);
					return true;
				}
			}
		}
		else
		{
			Log::Error("ioctl(..., FIONREAD, ...) failed with errno={} ({})"), errno, Strings::Convert<String>(strerror(errno)));
		}
		return false;
	}

	bool LinuxPipeHandle::Read(TArray<u8>& output)
	{
		if (!valid)
		{
			return false;
		}

		i32 bytesAvailable = 0;
		if (ioctl(readPipe, FIONREAD, &bytesAvailable) == 0)
		{
			if (bytesAvailable > 0)
			{
				const i32 initialSize = output.Size();
				output.Resize(initialSize + i32(bytesAvailable));
				const i32 bytesRead = read(readPipe, output.Data() + initialSize, bytesAvailable);
				if (bytesRead > 0)
				{
					if (bytesRead < bytesAvailable)
					{
						output.Resize(initialSize + i32(bytesRead));
					}
					return true;
				}
			}
		}
		else
		{
			Log::Error("ioctl(..., FIONREAD, ...) failed with errno={} ({})"), errno, Strings::Convert<String>(strerror(errno)));
		}
		return false;
	}

	bool LinuxPipeHandle::Write(const String& msg, String* outWritten)
	{
		if (data.IsEmpty() || !valid)
		{
			return false;
		}

		// Convert input to UTF8CHAR
		const u32 bytesAvailable = msg.Size();
		auto* buffer = new Char8[bytesAvailable + 2];
		for (u32 i = 0; i < bytesAvailable; i++)
		{
			buffer[i] = static_cast<Char8>(Char8>(msg[i]);
		}
		buffer[bytesAvailable] = static_cast<Char8>('\n');

		// write to pipe
		u32 bytesWritten = write(writePipe, buffer, bytesAvailable + 1);

		// Get written message
		if (outWritten)
		{
			Strings::ConvertTo<String, Char8>({buffer, bytesWritten}, *outWritten);
		}

		delete[] buffer;
		return (bytesWritten == bytesAvailable);
	}

	bool LinuxPipeHandle::Write(TSpan<const u8> data, i32* outWrittenLength)
	{
		if (data.IsEmpty() || !valid)
		{
			return false;
		}

		const u32 bytesWritten = write(writePipe, data.Data(), data.Size());

		if (outWrittenLength)
		{
			*outWrittenLength = static_cast<i32>(bytesWritten);
		}
		return bytesWritten == data.Size();
	}

	StringView LinuxPlatformProcess::GetExecutableFile()
	{
		static String path;
		if (path.empty())
		{
			TFixedString<PlatformMisc::GetMaxPathLength(), char> rawPath{};
			if (readlink("/proc/self/exe", rawPath.data(), rawPath.size() - 1) == -1)
			{
				// readlink() failed. Unreachable
				return {};
			}

			path = Strings::Convert<String>(
			    TStringView<char>{rawPath.data(), Strings::Length(rawPath.data())});
		}
		return path;
	}
	StringView LinuxPlatformProcess::GetExecutablePath()
	{
		return GetParentPath(GetExecutableFile());
	}

	StringView LinuxPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}

	void LinuxPlatformProcess::ShowFolder(StringView path)
	{
		if (!files::Exists(path))
		{
			return;
		}

		if (!files::IsFolder(path))
		{
			path = GetParentPath(path);
		}
		String fullPath{path};

		// launch file manager
		pid_t pid = fork();
		if (pid == 0)
		{
			exit(execl("/usr/bin/xdg-open", "xdg-open", fullPath.data(), (char*)0));
		}
	}

	bool LinuxPlatformProcess::CreatePipe(
	    PipeHandle& readPipe, PipeHandle& writePipe, bool writePipeLocal)
	{}

	void LinuxPlatformProcess::ClosePipe(PipeHandle& readPipe, PipeHandle& writePipe) {}

	String LinuxPlatformProcess::ReadPipe(const PipeHandle& readPipe) {}

	bool LinuxPlatformProcess::ReadPipe(const PipeHandle& readPipe, TArray<u8>& output) {}

	bool LinuxPlatformProcess::WritePipe(
	    const PipeHandle& writePipe, const String& msg, String* outWritten)
	{}

	bool LinuxPlatformProcess::WritePipe(
	    const PipeHandle& writePipe, TSpan<const u8> data, i32* outWrittenLength)
	{}
}    // namespace p::core
#endif
