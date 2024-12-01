// Copyright 2015-2024 Piperift - All rights reserved
#include "Pipe/Core/Subprocess.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Core/Optional.h"
#include "Pipe/Core/PlatformMisc.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"
#include "PipeArrays.h"


#if defined(_MSC_VER)
	#include <Windows.h>
	#include <io.h>
#else
extern char** environ;
#endif


namespace p
{
#if defined(_MSC_VER)
	struct SubprocessInfo
	{
		void* hProcess;
		void* hThread;
		u64 dwProcessId;
		u64 dwThreadId;
	};

	struct SubprocessSecurityAttributes
	{
		u64 nLength                = sizeof(SubprocessSecurityAttributes);
		void* lpSecurityDescriptor = nullptr;
		i32 bInheritHandle         = 1;
	};

	struct SubprocessStartupInfo
	{
		u64 cb              = sizeof(SubprocessStartupInfo);
		char* lpReserved    = nullptr;
		char* lpDesktop     = nullptr;
		char* lpTitle       = nullptr;
		u64 dwX             = 0;
		u64 dwY             = 0;
		u64 dwXSize         = 0;
		u64 dwYSize         = 0;
		u64 dwXCountChars   = 0;
		u64 dwYCountChars   = 0;
		u64 dwFillAttribute = 0;
		u64 dwFlags         = 0;
		u16 wShowWindow     = 0;
		u16 cbReserved2     = 0;
		u8* lpReserved2     = nullptr;
		void* hStdInput     = nullptr;
		void* hStdOutput    = nullptr;
		void* hStdError     = nullptr;
	};

	struct SubprocessOverlapped
	{
		uintptr_t Internal     = 0;
		uintptr_t InternalHigh = 0;
		union
		{
			struct
			{
				u64 Offset;
				u64 OffsetHigh;
			} DUMMYSTRUCTNAME;
			void* Pointer;
		} DUMMYUNIONNAME;

		void* hEvent = nullptr;


		SubprocessOverlapped() : Offset{0}, OffsetHigh{0} {}
	};
#endif


	Subprocess::Subprocess(Subprocess&& other) noexcept
	{
		p::Swap(cinFile, other.cinFile);
		p::Swap(coutFile, other.coutFile);
		p::Swap(cerrFile, other.cerrFile);
#if defined(_MSC_VER)
		p::Swap(hProcess, other.hProcess);
		p::Swap(hStdInput, other.hStdInput);
		p::Swap(hEventOutput, other.hEventOutput);
		p::Swap(hEventError, other.hEventError);
#else
		p::Swap(child, other.child);
		p::Swap(returnStatus, other.returnStatus);
#endif
		p::Swap(options, other.options);
		p::Swap(alive, other.alive);
	}

	Subprocess::~Subprocess()
	{
		if (HasFlag(options, SubprocessOptions::TerminateIfDestroyed))
		{
			TerminateProcess(this);
		}
		DestroyProcess(this);    // Make sure process is destroyed
	}


#if defined(_MSC_VER)
	const char* GetSystemErrorMessage(char* buffer, i32 size, i32 error)
	{
		P_Check(buffer && size);

		*buffer = '\0';
		if (error == 0)
		{
			error = ::GetLastError();
		}
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error,
		    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, size, nullptr);
		char* found = std::strchr(buffer, '\r');
		if (found)
		{
			*found = '\0';
		}
		found = std::strchr(buffer, '\n');
		if (found)
		{
			*found = '\0';
		}
		return buffer;
	}

	i32 RunProcessNamedPipeHelper(void** readPipe, void** writePipe)
	{
		const u64 pipeAccessInbound          = 0x00000001;
		const u64 fileFlagOverlapped         = 0x40000000;
		const u64 pipeTypeByte               = 0x00000000;
		const u64 pipeWait                   = 0x00000000;
		const u64 genericWrite               = 0x40000000;
		const u64 openExisting               = 3;
		const u64 fileAttributeNormal        = 0x00000080;
		const void* const invalidHandleValue = reinterpret_cast<void*>(~static_cast<iPtr>(0));
		SubprocessSecurityAttributes saAttr  = {sizeof(saAttr), nullptr, 1};
		char name[256]                       = {0};
		__declspec(thread) static long index = 0;
		const long unique                    = index++;

	#if _MSC_VER < 1900
		#pragma warning(push, 1)
		#pragma warning(disable:4996)
		_snprintf(name, sizeof(name) - 1, "\\\\.\\pipe\\sheredom_subprocess_h.%08lx.%08lx.%ld",
		    ::GetCurrentProcessId(), ::GetCurrentThreadId(), unique);
		#pragma warning(pop)
	#else
		snprintf(name, sizeof(name) - 1, R"(\\.\pipe\sheredom_subprocess_h.%08lx.%08lx.%ld)",
		    ::GetCurrentProcessId(), ::GetCurrentThreadId(), unique);
	#endif

		*readPipe = ::CreateNamedPipeA(name, pipeAccessInbound | fileFlagOverlapped,
		    pipeTypeByte | pipeWait, 1, 4096, 4096, 0,
		    reinterpret_cast<_SECURITY_ATTRIBUTES*>(&saAttr));

		if (invalidHandleValue == *readPipe)
		{
			return -1;
		}

		*writePipe =
		    ::CreateFileA(name, genericWrite, 0, reinterpret_cast<_SECURITY_ATTRIBUTES*>(&saAttr),
		        openExisting, fileAttributeNormal, nullptr);

		if (invalidHandleValue == *writePipe)
		{
			return -1;
		}

		return 0;
	}
#endif


	TOptional<Subprocess> RunProcessEx(TView<const char* const> command,
	    TView<const char* const> environment, SubprocessOptions options)
	{
		Subprocess instance;
		instance.options = options;

#if defined(_MSC_VER)
		i32 i, j;
		u64 flags                                = 0;
		static constexpr u64 startFUseStdHandles = 0x00000100;
		static constexpr u64 handleFlagInherit   = 0x00000001;
		static constexpr u64 createNoWindow      = 0x08000000;
		TString<char> commandCombined;
		TString<char> environmentCombined;

		SubprocessInfo processInfo;
		SubprocessSecurityAttributes saAttr;
		SubprocessStartupInfo startInfo;
		startInfo.dwFlags = startFUseStdHandles;

		if (HasFlag(options, SubprocessOptions::NoWindow))
		{
			flags |= createNoWindow;
		}

		for (i = 0; i < command.Size(); ++i)
		{
			auto commandStep       = command[i];
			const bool needQuoting = Strings::Contains(commandStep, "\t")
			                      || Strings::Contains(commandStep, '\v')
			                      || Strings::Contains(commandStep, ' ');
			if (needQuoting)
			{
				commandCombined.push_back('"');
			}

			for (j = 0; commandStep[j]; ++j)
			{
				switch (commandStep[j])
				{
					default: break;
					case '\\':
						if (commandStep[j + 1] == '"')
						{
							commandCombined.push_back('\\');
						}
						break;
					case '"': commandCombined.push_back('\\'); break;
				}

				commandCombined.push_back(commandStep[j]);
			}
			if (needQuoting)
			{
				commandCombined.push_back('"');
			}

			commandCombined.push_back(' ');
		}
		commandCombined.pop_back();    // Remove last space
		if (commandCombined.empty())
		{
			return {};
		}

		if (HasFlag(options, SubprocessOptions::InheritEnvironment))
		{
			if (!environment.IsEmpty())
			{
				return {};
			}
		}
		else
		{
			if (environment.IsEmpty())
			{
				environmentCombined.push_back('\0');
			}
			else
			{
				for (i = 0; i < environment.Size(); ++i)
				{
					environmentCombined.append(environment[i]);
					environmentCombined.push_back('\0');
				}
			}
		}

		void* readPipe  = nullptr;
		void* writePipe = nullptr;
		if (!::CreatePipe(
		        &readPipe, &writePipe, reinterpret_cast<_SECURITY_ATTRIBUTES*>(&saAttr), 0))
		{
			return {};
		}
		if (!::SetHandleInformation(writePipe, handleFlagInherit, 0))
		{
			return {};
		}
		const i32 writeDescriptor = _open_osfhandle(reinterpret_cast<iPtr>(writePipe), 0);
		if (writeDescriptor != -1)
		{
			instance.cinFile = _fdopen(writeDescriptor, "wb");

			if (!instance.cinFile)
			{
				return {};
			}
		}
		startInfo.hStdInput = readPipe;
		instance.hStdInput  = readPipe;

		if (HasFlag(options, SubprocessOptions::EnableAsync))
		{
			if (RunProcessNamedPipeHelper(&readPipe, &writePipe))
			{
				return {};
			}
		}
		else if (!::CreatePipe(
		             &readPipe, &writePipe, reinterpret_cast<_SECURITY_ATTRIBUTES*>(&saAttr), 0))
		{
			return {};
		}
		if (!::SetHandleInformation(readPipe, handleFlagInherit, 0))
		{
			return {};
		}
		const i32 readDescriptor = _open_osfhandle(reinterpret_cast<iPtr>(readPipe), 0);
		if (readDescriptor != -1)
		{
			instance.coutFile = _fdopen(readDescriptor, "rb");
			if (!instance.coutFile)
			{
				return {};
			}
		}
		startInfo.hStdOutput = writePipe;

		if (HasFlag(options, SubprocessOptions::CombinedOutErr))
		{
			instance.cerrFile   = instance.coutFile;
			startInfo.hStdError = startInfo.hStdOutput;
		}
		else
		{
			if (HasFlag(options, SubprocessOptions::EnableAsync))
			{
				if (RunProcessNamedPipeHelper(&readPipe, &writePipe))
				{
					return {};
				}
			}
			else
			{
				if (!::CreatePipe(
				        &readPipe, &writePipe, reinterpret_cast<_SECURITY_ATTRIBUTES*>(&saAttr), 0))
				{
					return {};
				}
			}
			if (!::SetHandleInformation(readPipe, handleFlagInherit, 0))
			{
				return {};
			}
			const i32 errorDescriptor = _open_osfhandle(reinterpret_cast<iPtr>(readPipe), 0);
			if (errorDescriptor != -1)
			{
				instance.cerrFile = ::_fdopen(errorDescriptor, "rb");
				if (!instance.cerrFile)
				{
					return {};
				}
			}
			startInfo.hStdError = writePipe;
		}

		if (HasFlag(options, SubprocessOptions::EnableAsync))
		{
			instance.hEventOutput =
			    CreateEventA(reinterpret_cast<_SECURITY_ATTRIBUTES*>(&saAttr), 1, 1, nullptr);
			instance.hEventError =
			    CreateEventA(reinterpret_cast<_SECURITY_ATTRIBUTES*>(&saAttr), 1, 1, nullptr);
		}
		else
		{
			instance.hEventOutput = nullptr;
			instance.hEventError  = nullptr;
		}

		if (!::CreateProcessA(nullptr,
		        commandCombined.data(),        // command line
		        nullptr,                       // process security attributes
		        nullptr,                       // primary thread security attributes
		        1,                             // handles are inherited
		        flags,                         // creation flags
		        environmentCombined.data(),    // used environment
		        nullptr,                       // use parent's current directory
		        reinterpret_cast<_STARTUPINFOA*>(&startInfo),    // STARTUPINFO pointer
		        reinterpret_cast<_PROCESS_INFORMATION*>(&processInfo)))
		{
			DWORD errorCode = GetLastError();
			char errorMessage[512];
			GetSystemErrorMessage(errorMessage, 512, errorCode);
			Error("RunProcess failed: {} (0x{:08x})", errorMessage, errorCode);
			return {};
		}

		instance.hProcess = processInfo.hProcess;

		// We don't need the handle of the primary thread in the called process.
		::CloseHandle(processInfo.hThread);

		if (startInfo.hStdOutput)
		{
			::CloseHandle(startInfo.hStdOutput);

			if (startInfo.hStdError != startInfo.hStdOutput)
			{
				::CloseHandle(startInfo.hStdError);
			}
		}

		instance.alive = true;
#else
		TArray<const char*> posixCommand;
		posixCommand.Append(command);
		posixCommand.Add(nullptr);    // End in nullptr following posix API

		TArray<const char*> posixEnvironment;
		posixEnvironment.Append(environment);
		posixEnvironment.Add(nullptr);    // End in nullptr following posix API

		i32 stdinfd[2];
		i32 stdoutfd[2];
		i32 stderrfd[2];
		pid_t child;
		posix_spawn_file_actions_t actions;

		char* const* usedEnvironment;
		if (HasFlag(options, SubprocessOptions::InheritEnvironment))
		{
			if (!environment.IsEmpty())
			{
				return {};
			}
			else
			{
				usedEnvironment = environ;
			}
		}
		else
		{
	#ifdef __clang__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wcast-qual"
		#pragma clang diagnostic ignored "-Wold-style-cast"
	#endif
			usedEnvironment = (char* const*)posixEnvironment.Data();
	#ifdef __clang__
		#pragma clang diagnostic pop
	#endif
		}

		if (pipe(stdinfd) != 0)
		{
			return {};
		}

		if (pipe(stdoutfd) != 0)
		{
			return {};
		}

		if (HasFlag(options, SubprocessOptions::CombinedOutErr))
		{
			if (pipe(stderrfd) != 0)
			{
				return {};
			}
		}

		if (0 != posix_spawn_file_actions_init(&actions))
		{
			return {};
		}

		// Close the stdin write end
		if (0 != posix_spawn_file_actions_addclose(&actions, stdinfd[1]))
		{
			posix_spawn_file_actions_destroy(&actions);
			return {};
		}

		// Map the read end to stdin
		if (0 != posix_spawn_file_actions_adddup2(&actions, stdinfd[0], STDIN_FILENO))
		{
			posix_spawn_file_actions_destroy(&actions);
			return {};
		}

		// Close the stdout read end
		if (0 != posix_spawn_file_actions_addclose(&actions, stdoutfd[0]))
		{
			posix_spawn_file_actions_destroy(&actions);
			return {};
		}

		// Map the write end to stdout
		if (0 != posix_spawn_file_actions_adddup2(&actions, stdoutfd[1], STDOUT_FILENO))
		{
			posix_spawn_file_actions_destroy(&actions);
			return {};
		}

		if (HasFlag(options, SubprocessOptions::CombinedOutErr))
		{
			if (0 != posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO))
			{
				posix_spawn_file_actions_destroy(&actions);
				return {};
			}
		}
		else
		{
			// Close the stderr read end
			if (0 != posix_spawn_file_actions_addclose(&actions, stderrfd[0]))
			{
				posix_spawn_file_actions_destroy(&actions);
				return {};
			}
			// Map the write end to stdout
			if (0 != posix_spawn_file_actions_adddup2(&actions, stderrfd[1], STDERR_FILENO))
			{
				posix_spawn_file_actions_destroy(&actions);
				return {};
			}
		}

	#ifdef __clang__
		#pragma clang diagnostic push
		#pragma clang diagnostic ignored "-Wcast-qual"
		#pragma clang diagnostic ignored "-Wold-style-cast"
	#endif
		if (HasFlag(options, SubprocessOptions::SearchUserPath))
		{
			if (posix_spawnp(&child, posixCommand[0], &actions, nullptr,
			        (char* const*)posixCommand.Data(), usedEnvironment)
			    != 0)
			{
				posix_spawn_file_actions_destroy(&actions);
				return {};
			}
		}
		else
		{
			if (posix_spawn(&child, posixCommand[0], &actions, nullptr,
			        (char* const*)posixCommand.Data(), usedEnvironment)
			    != 0)
			{
				posix_spawn_file_actions_destroy(&actions);
				return {};
			}
		}
	#ifdef __clang__
		#pragma clang diagnostic pop
	#endif

		// Close the stdin read end
		close(stdinfd[0]);
		// Store the stdin write end
		instance.cinFile = fdopen(stdinfd[1], "wb");

		// Close the stdout write end
		close(stdoutfd[1]);
		// Store the stdout read end
		instance.coutFile = fdopen(stdoutfd[0], "rb");

		if (HasFlag(options, SubprocessOptions::CombinedOutErr))
		{
			instance.cerrFile = instance.coutFile;
		}
		else
		{
			// Close the stderr write end
			close(stderrfd[1]);
			// Store the stderr read end
			instance.cerrFile = fdopen(stderrfd[0], "rb");
		}

		// Store the child's pid
		instance.child = child;

		instance.alive = true;

		posix_spawn_file_actions_destroy(&actions);
#endif
		return {Move(instance)};
	}

	i32 WaitProcess(Subprocess* process, i32* outReturnCode)
	{
#if defined(_MSC_VER)
		const u64 infinite = 0xFFFFFFFF;

		if (process->cinFile)
		{
			fclose(process->cinFile);
			process->cinFile = nullptr;
		}

		if (process->hStdInput)
		{
			::CloseHandle(process->hStdInput);
			process->hStdInput = nullptr;
		}

		::WaitForSingleObject(process->hProcess, infinite);

		if (outReturnCode)
		{
			if (!::GetExitCodeProcess(process->hProcess, reinterpret_cast<::DWORD*>(outReturnCode)))
			{
				return -1;
			}
		}

		process->alive = false;
		return 0;
#else
		i32 status;

		if (process->cinFile)
		{
			fclose(process->cinFile);
			process->cinFile = nullptr;
		}

		if (process->child)
		{
			if (process->child != waitpid(process->child, &status, 0))
			{
				return -1;
			}

			process->child = 0;

			if (WIFEXITED(status))
			{
				process->returnStatus = WEXITSTATUS(status);
			}
			else
			{
				process->returnStatus = EXIT_FAILURE;
			}

			process->alive = false;
		}

		if (outReturnCode)
		{
			*outReturnCode = process->returnStatus;
		}

		return 0;
#endif
	}


	i32 DestroyProcess(Subprocess* process)
	{
		if (process->cinFile)
		{
			fclose(process->cinFile);
			process->cinFile = nullptr;
		}

		if (process->coutFile)
		{
			fclose(process->coutFile);
			if (process->coutFile != process->cerrFile)
			{
				fclose(process->cerrFile);
			}

			process->coutFile = nullptr;
			process->cerrFile = nullptr;
		}

#if defined(_MSC_VER)
		if (process->hProcess)
		{
			CloseHandle(process->hProcess);
			process->hProcess = nullptr;

			if (process->hStdInput)
			{
				CloseHandle(process->hStdInput);
				process->hStdInput = nullptr;
			}
			if (process->hEventOutput)
			{
				CloseHandle(process->hEventOutput);
				process->hEventOutput = nullptr;
			}
			if (process->hEventError)
			{
				CloseHandle(process->hEventError);
				process->hEventError = nullptr;
			}
		}
#endif
		return 0;
	}

	i32 TerminateProcess(Subprocess* process)
	{
#if defined(_MSC_VER)
		if (process->hProcess)
		{
			u32 killed_process_exit_code;
			i32 success_terminate;
			i32 windows_call_result;

			killed_process_exit_code = 99;
			windows_call_result = ::TerminateProcess(process->hProcess, killed_process_exit_code);
			success_terminate   = (windows_call_result == 0) ? 1 : 0;
			process->hProcess   = nullptr;
			return success_terminate;
		}
#else
		if (process->child != 0)
		{
			i32 result;
			result         = kill(process->child, 9);
			process->child = 0;
			return result;
		}
#endif
		return false;
	}

	unsigned ReadProcessCout(Subprocess* process, char* buffer, u32 size)
	{
#if defined(_MSC_VER)
		void* handle;
		u64 bytesRead = 0;
		SubprocessOverlapped overlapped;
		overlapped.hEvent = process->hEventOutput;

		handle = reinterpret_cast<void*>(_get_osfhandle(::_fileno(process->coutFile)));

		if (!::ReadFile(handle, buffer, size, (::DWORD*)&bytesRead,
		        reinterpret_cast<_OVERLAPPED*>(&overlapped)))
		{
			const u64 errorIoPending = 997;
			u64 error                = GetLastError();

			// Means we've got an async read!
			if (error == errorIoPending)
			{
				if (!::GetOverlappedResult(handle, reinterpret_cast<_OVERLAPPED*>(&overlapped),
				        (::DWORD*)&bytesRead, 1))
				{
					const u64 errorIoIncomplete = 996;
					const u64 errorHandleEOF    = 38;
					error                       = GetLastError();

					if ((error != errorIoIncomplete) && (error != errorHandleEOF))
					{
						return 0;
					}
				}
			}
		}

		return static_cast<unsigned>(bytesRead);
#else
		const i32 fd            = fileno(process->coutFile);
		const ssize_t bytesRead = read(fd, buffer, size);

		if (bytesRead < 0)
		{
			return 0;
		}

		return static_cast<unsigned>(bytesRead);
#endif
	}

	unsigned ReadProcessCerr(Subprocess* process, char* buffer, unsigned size)
	{
#if defined(_MSC_VER)
		void* handle;
		u64 bytesRead = 0;
		SubprocessOverlapped overlapped;
		overlapped.hEvent = process->hEventError;

		handle = reinterpret_cast<void*>(_get_osfhandle(::_fileno(process->cerrFile)));

		if (!::ReadFile(handle, buffer, size, (::DWORD*)&bytesRead,
		        reinterpret_cast<_OVERLAPPED*>(&overlapped)))
		{
			const u64 errorIoPending = 997;
			u64 error                = GetLastError();

			// Means we've got an async read!
			if (error == errorIoPending)
			{
				if (!::GetOverlappedResult(handle, reinterpret_cast<_OVERLAPPED*>(&overlapped),
				        (::DWORD*)&bytesRead, 1))
				{
					const u64 errorIoIncomplete = 996;
					const u64 errorHandleEOF    = 38;
					error                       = GetLastError();

					if ((error != errorIoIncomplete) && (error != errorHandleEOF))
					{
						return 0;
					}
				}
			}
		}

		return static_cast<unsigned>(bytesRead);
#else
		const i32 fd            = fileno(process->cerrFile);
		const ssize_t bytesRead = read(fd, buffer, size);

		if (bytesRead < 0)
		{
			return 0;
		}

		return static_cast<unsigned>(bytesRead);
#endif
	}

	bool IsAlive(Subprocess* process)
	{
		if (!process->alive)
		{
			return false;
		}

#if defined(_MSC_VER)
		constexpr u64 zero          = 0x0;
		constexpr u64 wait_object_0 = 0x00000000L;

		process->alive = wait_object_0 != WaitForSingleObject(process->hProcess, zero);
#else
		i32 status;
		process->alive = waitpid(process->child, &status, WNOHANG) == 0;

		// If the process was successfully waited on we need to cleanup now.
		if (!process->alive)
		{
			if (WIFEXITED(status))
			{
				process->returnStatus = WEXITSTATUS(status);
			}
			else
			{
				process->returnStatus = EXIT_FAILURE;
			}

			// Since we've already successfully waited on the process, we need to wipe
			// the child now.
			process->child = 0;

			if (WaitProcess(process, nullptr))
			{
				return -1;
			}
		}
#endif
		return process->alive;
	}

	FILE* GetProcessCin(const Subprocess* process)
	{
		return process->cinFile;
	}

	FILE* GetProcessCout(const Subprocess* process)
	{
		return process->coutFile;
	}

	FILE* GetProcessCerr(const Subprocess* process)
	{
		if (process->coutFile != process->cerrFile)
		{
			return process->cerrFile;
		}
		else
		{
			return nullptr;
		}
	}
}    // namespace p
