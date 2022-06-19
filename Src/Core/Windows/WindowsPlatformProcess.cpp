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

	bool CreatePipe(void*& readPipe, void*& writePipe, bool writePipeLocal)
	{
		SECURITY_ATTRIBUTES Attr = {sizeof(SECURITY_ATTRIBUTES), nullptr, true};

		if (!::CreatePipe(&readPipe, &writePipe, &Attr, 0))
		{
			return false;
		}

		if (!::SetHandleInformation(writePipeLocal ? writePipe : readPipe, HANDLE_FLAG_INHERIT, 0))
		{
			return false;
		}

		return true;
	}

	void ClosePipe(void* readPipe, void* writePipe)
	{
		if (readPipe != nullptr && readPipe != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(readPipe);
		}
		if (writePipe != nullptr && writePipe != INVALID_HANDLE_VALUE)
		{
			::CloseHandle(writePipe);
		}
	}

	String ReadPipe(void* readPipe)
	{
		String output;
		// Note: String becomes corrupted when more than one byte per character and all bytes are
		// not available
		u32 bytesAvailable = 0;
		if (::PeekNamedPipe(ReadPipe, nullptr, 0, nullptr, (::DWORD*)&bytesAvailable, nullptr)
		    && (bytesAvailable > 0))
		{
			auto* buffer  = new Char8[bytesAvailable + 1];
			u32 bytesRead = 0;
			if (::ReadFile(ReadPipe, buffer, bytesAvailable, (::DWORD*)&bytesRead, nullptr))
			{
				if (bytesRead > 0)
				{
					// buffer[bytesRead] = (Char8)'\0';
					Strings::ConvertTo<String, Char8>({buffer, bytesRead}, output);
				}
			}
			delete[] buffer;
		}
		return output;
	}

	bool ReadPipe(void* readPipe, TArray<u8>& output)
	{
		u32 bytesAvailable = 0;
		if (::PeekNamedPipe(readPipe, nullptr, 0, nullptr, (::DWORD*)&bytesAvailable, nullptr)
		    && (bytesAvailable > 0))
		{
			// Note: Can be improved since resize default
			// initializes vaues and this is not needed
			output.Resize(i32(bytesAvailable));
			u32 bytesRead = 0;
			if (::ReadFile(readPipe, output.Data(), bytesAvailable, (::DWORD*)&bytesRead, nullptr))
			{
				if (bytesRead < bytesAvailable)
				{
					output.Resize(i32(bytesRead));
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

	bool WritePipe(void* writePipe, const String& msg, String* outWritten = nullptr)
	{
		// If there is not a message or WritePipe is null
		if (msg.empty() || WritePipe == nullptr)
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
		    !!WriteFile(WritePipe, buffer, bytesAvailable + 1, (::DWORD*)&bytesWritten, nullptr);

		// Get written message
		if (outWritten)
		{
			// buffer[bytesWritten] = (Char8)'\0';
			Strings::ConvertTo<String, Char8>({buffer, bytesWritten}, *outWritten);
		}
		delete[] buffer;
		return isWritten;
	}

	bool WritePipe(void* writePipe, TSpan<const u8> data, i32* outWrittenLength = nullptr)
	{
		// if there is not a message or WritePipe is null
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
			*outWrittenLength = (i32)bytesWritten;
		}

		return isWritten;
	}
}    // namespace p::core
#endif
