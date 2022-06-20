// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Core/Platform.h"
#if PLATFORM_WINDOWS
#	include "Pipe/Files/Paths.h"
#	include "Pipe/Files/Files.h"
#	include "Pipe/Core/Windows/WindowsPlatformProcess.h"
#	include "Pipe/Core/String.h"

#	include <Windows.h>


namespace p::core
{
	WindowsPipeHandle::WindowsPipeHandle(bool writePipeLocal)
	{
		SECURITY_ATTRIBUTES Attr = {sizeof(SECURITY_ATTRIBUTES), nullptr, true};

		if (!::CreatePipe(&readPipe, &writePipe, &Attr, 0))
		{
			return false;
		}

		return ::SetHandleInformation(
		    writePipeLocal ? writePipe : readPipe, HANDLE_FLAG_INHERIT, 0);
	}

	WindowsPipeHandle::WindowsPipeHandle(WindowsPipeHandle&& other) noexcept
	    : readPipe{other.readPipe}, writePipe{other.writePipe}
	{
		other.readPipe  = nullptr;
		other.writePipe = nullptr;
	}

	void WindowsPipeHandle::Close()
	{
		if (readPipe != nullptr && readPipe != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(readPipe);
			readPipe = nullptr;
		}
		if (writePipe != nullptr && writePipe != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(writePipe);
			writePipe = nullptr;
		}
	}

	bool WindowsPipeHandle::Read(String& output)
	{
		if (readPipe == nullptr)
		{
			// Pipe is invalid. Nothing to read
			return false;
		}

		// Note: String becomes corrupted when more than one byte per character and all bytes are
		// not available
		u32 bytesAvailable = 0;
		if (::PeekNamedPipe(readPipe, nullptr, 0, nullptr, (::DWORD*)&bytesAvailable, nullptr)
		    && (bytesAvailable > 0))
		{
			auto* buffer  = new Char8[bytesAvailable + 1];
			u32 bytesRead = 0;
			if (::ReadFile(readPipe, buffer, bytesAvailable, (::DWORD*)&bytesRead, nullptr))
			{
				if (bytesRead > 0)
				{
					// buffer[bytesRead] = (Char8)'\0';
					Strings::ConvertTo<String, Char8>({buffer, bytesRead}, output);
				}
				return true;
			}
			delete[] buffer;
		}
		return false;
	}

	bool WindowsPipeHandle::Read(TArray<u8>& output)
	{
		if (readPipe == nullptr)
		{
			// Pipe is invalid. Nothing to read
			return false;
		}

		u32 bytesAvailable = 0;
		if (::PeekNamedPipe(readPipe, nullptr, 0, nullptr, (::DWORD*)&bytesAvailable, nullptr)
		    && (bytesAvailable > 0))
		{
			const i32 initialSize = output.Size();
			// Note: Can be improved since resize default
			// initializes vaues and this is not needed
			output.Resize(initialSize + i32(bytesAvailable));
			u32 bytesRead = 0;
			if (::ReadFile(readPipe, output.Data() + initialSize, bytesAvailable, (::DWORD*)&bytesRead, nullptr))
			{
				if (bytesRead < bytesAvailable)
				{
					output.Resize(initialSize + i32(bytesRead));
				}
				return true;
			}
			else
			{
				output.Empty();
			}
		}
		return false;
	}

	bool WindowsPipeHandle::Write(const String& msg, String* outWritten)
	{
		if (msg.empty() || writePipe == nullptr)
		{
			return false;
		}

		// Convert input to Char8
		u32 bytesAvailable = msg.size();
		auto* buffer       = new Char8[bytesAvailable + 2];
		for (u32 i = 0; i < bytesAvailable; i++)
		{
			buffer[i] = static_cast<Char8>(msg[i]);
		}
		buffer[bytesAvailable] = static_cast<Char8>('\n');

		// Write to pipe
		u32 bytesWritten = 0;
		bool isWritten =
		    !!WriteFile(writePipe, buffer, bytesAvailable + 1, (::DWORD*)&bytesWritten, nullptr);

		// Get written message
		if (outWritten)
		{
			// buffer[bytesWritten] = (Char8)'\0';
			Strings::ConvertTo<String, Char8>({buffer, bytesWritten}, *outWritten);
		}
		delete[] buffer;
		return isWritten;
	}

	bool WindowsPipeHandle::Write(TSpan<const u8> data, i32* outWrittenLength)
	{
		if (data.Size() == 0 || writePipe == nullptr)
		{
			return false;
		}

		// write to pipe
		u32 bytesWritten = 0;
		bool isWritten =
		    !!WriteFile(writePipe, data.Data(), data.Size(), (::DWORD*)&bytesWritten, nullptr);

		// Get written Data Length
		if (outWrittenLength)
		{
			*outWrittenLength = static_cast<i32>(bytesWritten);
		}
		return isWritten;
	}


	template<typename StringType, typename TStringGetterFunc>
	StringType GetStringFromWindowsAPI(TStringGetterFunc stringGetter, int initialSize = MAX_PATH)
	{
		if (initialSize <= 0)
		{
			initialSize = MAX_PATH;
		}

		StringType result(initialSize, 0);
		for (;;)
		{
			auto length = stringGetter(result.data(), result.length());
			if (length == 0)
			{
				return {};
			}

			if (length < result.length() - 1)
			{
				result.resize(length);
				result.shrink_to_fit();
				return result;
			}

			result.resize(result.length() * 2);
		}
	}

	StringView WindowsPlatformProcess::GetExecutableFile()
	{
		static const auto filePath = GetStringFromWindowsAPI<String>([](TChar* buffer, sizet size) {
#	if PLATFORM_TCHAR_IS_WCHAR
			return GetModuleFileNameW(nullptr, buffer, u32(size));
#	else
			return GetModuleFileNameA(nullptr, buffer, u32(size));
#	endif
		});
		return filePath;
	}

	StringView WindowsPlatformProcess::GetExecutablePath()
	{
		return GetParentPath(GetExecutableFile());
	}

	StringView WindowsPlatformProcess::GetBasePath()
	{
		return GetExecutablePath();
	}


	void WindowsPlatformProcess::ShowFolder(StringView path)
	{
		if (!files::Exists(path))
		{
			return;
		}

		if (files::IsFolder(path))
		{
			const String fullPath{path};
			::ShellExecuteA(nullptr, "explore", fullPath.data(), nullptr, nullptr, SW_SHOWNORMAL);
		}
		else if (files::IsFile(path))
		{
			String parameters = Strings::Format("/select,{}", path);
			::ShellExecuteA(
			    nullptr, "open", "explorer.exe", parameters.data(), nullptr, SW_SHOWNORMAL);
		}
	}
}    // namespace p::core
#endif
