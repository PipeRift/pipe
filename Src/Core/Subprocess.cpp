// Copyright 2015-2022 Piperift - All rights reserved
#include "Pipe/Core/Subprocess.h"

#include "Pipe/Core/Array.h"

#if defined(_MSC_VER)
#	include <Windows.h>
#	include <io.h>
#endif


namespace p::core
{
#if defined(_MSC_VER)
#	ifdef __clang__
#		pragma clang diagnostic push
#		pragma clang diagnostic ignored "-Wreserved-identifier"
#	endif
	using LPPROCESS_INFORMATION = struct _PROCESS_INFORMATION*;
	using LPSECURITY_ATTRIBUTES = struct _SECURITY_ATTRIBUTES*;
	using LPSTARTUPINFOA        = struct _STARTUPINFOA*;
	using LPOVERLAPPED          = struct _OVERLAPPED*;
#	ifdef __clang__
#		pragma clang diagnostic pop
#	endif

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
		u64 cb              = 0;
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

/*
	__declspec(dllimport) u64 __stdcall GetLastError(void);
	__declspec(dllimport) i32 __stdcall SetHandleInformation(void*, u64, u64);
	__declspec(dllimport) i32 __stdcall CreatePipe(void**, void**, LPSECURITY_ATTRIBUTES, u64);
	__declspec(dllimport) void* __stdcall CreateNamedPipeA(
	    const char*, u64, u64, u64, u64, u64, u64, LPSECURITY_ATTRIBUTES);
	__declspec(dllimport) i32 __stdcall ReadFile(void*, void*, u64, u64*, LPOVERLAPPED);
	__declspec(dllimport) u64 __stdcall GetCurrentProcessId(void);
	__declspec(dllimport) u64 __stdcall GetCurrentThreadId(void);
	__declspec(dllimport) void* __stdcall CreateFileA(
	    const char*, u64, u64, LPSECURITY_ATTRIBUTES, u64, u64, void*);
	__declspec(dllimport) void* __stdcall CreateEventA(
	    LPSECURITY_ATTRIBUTES, int, int, const char*);
	__declspec(dllimport) i32 __stdcall CreateProcessA(const char*, char*, LPSECURITY_ATTRIBUTES,
	    LPSECURITY_ATTRIBUTES, int, u64, void*, const char*, LPSTARTUPINFOA, LPPROCESS_INFORMATION);
	__declspec(dllimport) i32 __stdcall CloseHandle(void*);
	__declspec(dllimport) u64 __stdcall WaitForSingleObject(void*, u64);
	__declspec(dllimport) i32 __stdcall GetExitCodeProcess(void*, u64* lpExitCode);
	__declspec(dllimport) i32 __stdcall TerminateProcess(void*, unsigned int);
	__declspec(dllimport) u64 __stdcall WaitForMultipleObjects(u64, void* const*, int, u64);
	__declspec(dllimport) i32 __stdcall GetOverlappedResult(void*, LPOVERLAPPED, u64*, int);

#	if defined(_DLL) && (_DLL == 1)
#		define SUBPROCESS_DLLIMPORT __declspec(dllimport)
#	else
#		define SUBPROCESS_DLLIMPORT
#	endif

#	ifdef __clang__
#		pragma clang diagnostic push
#		pragma clang diagnostic ignored "-Wreserved-identifier"
#	endif

	SUBPROCESS_DLLIMPORT i32 __cdecl _fileno(FILE*);
	SUBPROCESS_DLLIMPORT i32 __cdecl _open_osfhandle(iPtr, int);
	SUBPROCESS_DLLIMPORT iPtr __cdecl _get_osfhandle(int);

	void* __cdecl _alloca(sizet);

#	ifdef __clang__
#		pragma clang diagnostic pop
#	endif*/
#endif


#if defined(_MSC_VER)
	i32 CreateProcessNamedPipeHelper(void** rd, void** wr)
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

#	if _MSC_VER < 1900
#		pragma warning(push, 1)
#		pragma warning(disable:4996)
		_snprintf(name, sizeof(name) - 1, "\\\\.\\pipe\\sheredom_subprocess_h.%08lx.%08lx.%ld",
		    ::GetCurrentProcessId(), ::GetCurrentThreadId(), unique);
#		pragma warning(pop)
#	else
		snprintf(name, sizeof(name) - 1, R"(\\.\pipe\sheredom_subprocess_h.%08lx.%08lx.%ld)",
		    ::GetCurrentProcessId(), ::GetCurrentThreadId(), unique);
#	endif

		*rd = ::CreateNamedPipeA(name, pipeAccessInbound | fileFlagOverlapped,
		    pipeTypeByte | pipeWait, 1, 4096, 4096, 0,
		    reinterpret_cast<LPSECURITY_ATTRIBUTES>(&saAttr));

		if (invalidHandleValue == *rd)
		{
			return -1;
		}

		*wr = ::CreateFileA(name, genericWrite, 0, reinterpret_cast<LPSECURITY_ATTRIBUTES>(&saAttr),
		    openExisting, fileAttributeNormal, nullptr);

		if (invalidHandleValue == *wr)
		{
			return -1;
		}

		return 0;
	}
#endif


	i32 CreateProcessEx(TSpan<const char*> command, TSpan<const char*> environment,
	    Subprocess* outProcess, SubprocessOptions options)
	{
#if defined(_MSC_VER)
		i32 fd;
		void *rd, *wr;
		char* commandCombined;
		sizet len;
		i32 i, j;
		i32 need_quoting;
		u64 flags                     = 0;
		const u64 startFUseStdHandles = 0x00000100;
		const u64 handleFlagInherit   = 0x00000001;
		const u64 createNoWindow      = 0x08000000;
		SubprocessInfo processInfo;
		SubprocessSecurityAttributes saAttr;
		char* usedEnvironment = nullptr;
		SubprocessStartupInfo startInfo;

		startInfo.cb      = sizeof(startInfo);
		startInfo.dwFlags = startFUseStdHandles;

		if (HasFlag(options, SubprocessOptions::NoWindow))
		{
			flags |= createNoWindow;
		}

		if (!HasFlag(options, SubprocessOptions::InheritEnvironment))
		{
			if (environment.IsEmpty())
			{
				usedEnvironment = const_cast<char*>("\0\0");
			}
			else
			{
				// We always end with two null terminators.
				len = 2;

				for (i = 0; environment[i]; ++i)
				{
					for (j = 0; '\0' != environment[i][j]; ++j)
					{
						len++;
					}

					// For the null terminator too.
					len++;
				}

				usedEnvironment = static_cast<char*>(::_alloca(len));

				// Re-use len for the insertion position
				len = 0;

				for (i = 0; environment[i]; ++i)
				{
					for (j = 0; '\0' != environment[i][j]; ++j)
					{
						usedEnvironment[len++] = environment[i][j];
					}

					usedEnvironment[len++] = '\0';
				}

				// End with the two null terminators.
				usedEnvironment[len++] = '\0';
				usedEnvironment[len++] = '\0';
			}
		}
		else
		{
			if (!environment.IsEmpty())
			{
				return -1;
			}
		}

		if (!::CreatePipe(&rd, &wr, reinterpret_cast<LPSECURITY_ATTRIBUTES>(&saAttr), 0))
		{
			return -1;
		}

		if (!::SetHandleInformation(wr, handleFlagInherit, 0))
		{
			return -1;
		}

		fd = _open_osfhandle(reinterpret_cast<iPtr>(wr), 0);

		if (-1 != fd)
		{
			outProcess->inFile = _fdopen(fd, "wb");

			if (nullptr == outProcess->inFile)
			{
				return -1;
			}
		}

		startInfo.hStdInput = rd;

		if (HasFlag(options, SubprocessOptions::EnableAsync))
		{
			if (CreateProcessNamedPipeHelper(&rd, &wr))
			{
				return -1;
			}
		}
		else
		{
			if (!CreatePipe(&rd, &wr, reinterpret_cast<LPSECURITY_ATTRIBUTES>(&saAttr), 0))
			{
				return -1;
			}
		}

		if (!SetHandleInformation(rd, handleFlagInherit, 0))
		{
			return -1;
		}

		fd = _open_osfhandle(reinterpret_cast<iPtr>(rd), 0);

		if (-1 != fd)
		{
			outProcess->coutFile = _fdopen(fd, "rb");

			if (nullptr == outProcess->coutFile)
			{
				return -1;
			}
		}

		startInfo.hStdOutput = wr;

		if (HasFlag(options, SubprocessOptions::CombinedOutErr))
		{
			outProcess->cerrFile = outProcess->coutFile;
			startInfo.hStdError  = startInfo.hStdOutput;
		}
		else
		{
			if (HasFlag(options, SubprocessOptions::EnableAsync))
			{
				if (CreateProcessNamedPipeHelper(&rd, &wr))
				{
					return -1;
				}
			}
			else
			{
				if (!CreatePipe(&rd, &wr, reinterpret_cast<LPSECURITY_ATTRIBUTES>(&saAttr), 0))
				{
					return -1;
				}
			}

			if (!SetHandleInformation(rd, handleFlagInherit, 0))
			{
				return -1;
			}

			fd = _open_osfhandle(reinterpret_cast<iPtr>(rd), 0);

			if (-1 != fd)
			{
				outProcess->cerrFile = ::_fdopen(fd, "rb");

				if (!outProcess->cerrFile)
				{
					return -1;
				}
			}

			startInfo.hStdError = wr;
		}

		if (HasFlag(options, SubprocessOptions::EnableAsync))
		{
			outProcess->hEventOutput =
			    CreateEventA(reinterpret_cast<LPSECURITY_ATTRIBUTES>(&saAttr), 1, 1, nullptr);
			outProcess->hEventError =
			    CreateEventA(reinterpret_cast<LPSECURITY_ATTRIBUTES>(&saAttr), 1, 1, nullptr);
		}
		else
		{
			outProcess->hEventOutput = nullptr;
			outProcess->hEventError  = nullptr;
		}

		// Combine command together into a single string
		len = 0;
		for (i = 0; command[i]; i++)
		{
			// for the trailing \0
			len++;

			// Quote the argument if it has a space in it
			if (strpbrk(command[i], "\t\v ") != nullptr)
				len += 2;

			for (j = 0; '\0' != command[i][j]; j++)
			{
				switch (command[i][j])
				{
					default: break;
					case '\\':
						if (command[i][j + 1] == '"')
						{
							len++;
						}

						break;
					case '"': len++; break;
				}
				len++;
			}
		}

		commandCombined = static_cast<char*>(::_alloca(len));

		if (!commandCombined)
		{
			return -1;
		}

		// Gonna re-use len to store the write index into commandCombined
		len = 0;

		for (i = 0; command[i]; i++)
		{
			if (0 != i)
			{
				commandCombined[len++] = ' ';
			}

			need_quoting = strpbrk(command[i], "\t\v ") != nullptr;
			if (need_quoting)
			{
				commandCombined[len++] = '"';
			}

			for (j = 0; '\0' != command[i][j]; j++)
			{
				switch (command[i][j])
				{
					default: break;
					case '\\':
						if (command[i][j + 1] == '"')
						{
							commandCombined[len++] = '\\';
						}

						break;
					case '"': commandCombined[len++] = '\\'; break;
				}

				commandCombined[len++] = command[i][j];
			}
			if (need_quoting)
			{
				commandCombined[len++] = '"';
			}
		}

		commandCombined[len] = '\0';

		if (!CreateProcessA(nullptr,
		        commandCombined,    // command line
		        nullptr,            // process security attributes
		        nullptr,            // primary thread security attributes
		        1,                  // handles are inherited
		        flags,              // creation flags
		        usedEnvironment,    // used environment
		        nullptr,            // use parent's current directory
		        reinterpret_cast<LPSTARTUPINFOA>(&startInfo),    // STARTUPINFO pointer
		        reinterpret_cast<LPPROCESS_INFORMATION>(&processInfo)))
		{
			return -1;
		}

		outProcess->hProcess = processInfo.hProcess;

		outProcess->hStdInput = startInfo.hStdInput;

		// We don't need the handle of the primary thread in the called process.
		CloseHandle(processInfo.hThread);

		if (nullptr != startInfo.hStdOutput)
		{
			CloseHandle(startInfo.hStdOutput);

			if (startInfo.hStdError != startInfo.hStdOutput)
			{
				CloseHandle(startInfo.hStdError);
			}
		}

		outProcess->alive = true;

		return 0;
#else
		i32 stdinfd[2];
		i32 stdoutfd[2];
		i32 stderrfd[2];
		pid_t child;
		extern char** environ;
		char* empty_environment[1] = {nullptr};
		posix_spawn_file_actions_t actions;
		char* const* usedEnvironment;

		if (HasFlag(options, SubprocessOptions::InheritEnvironment))
		{
			if (nullptr != environment)
			{
				return -1;
			}
		}

		if (0 != pipe(stdinfd))
		{
			return -1;
		}

		if (0 != pipe(stdoutfd))
		{
			return -1;
		}

		if (SubprocessOptions::CombinedOutErr != (options & SubprocessOptions::CombinedOutErr))
		{
			if (0 != pipe(stderrfd))
			{
				return -1;
			}
		}

		if (environment)
		{
#	ifdef __clang__
#		pragma clang diagnostic push
#		pragma clang diagnostic ignored "-Wcast-qual"
#		pragma clang diagnostic ignored "-Wold-style-cast"
#	endif
			usedEnvironment = (char* const*)environment;
#	ifdef __clang__
#		pragma clang diagnostic pop
#	endif
		}
		else if (HasFlag(options, subprocess_option_inherit_environment))
		{
			usedEnvironment = environ;
		}
		else
		{
			usedEnvironment = empty_environment;
		}

		if (0 != posix_spawn_file_actions_init(&actions))
		{
			return -1;
		}

		// Close the stdin write end
		if (0 != posix_spawn_file_actions_addclose(&actions, stdinfd[1]))
		{
			posix_spawn_file_actions_destroy(&actions);
			return -1;
		}

		// Map the read end to stdin
		if (0 != posix_spawn_file_actions_adddup2(&actions, stdinfd[0], STDIN_FILENO))
		{
			posix_spawn_file_actions_destroy(&actions);
			return -1;
		}

		// Close the stdout read end
		if (0 != posix_spawn_file_actions_addclose(&actions, stdoutfd[0]))
		{
			posix_spawn_file_actions_destroy(&actions);
			return -1;
		}

		// Map the write end to stdout
		if (0 != posix_spawn_file_actions_adddup2(&actions, stdoutfd[1], STDOUT_FILENO))
		{
			posix_spawn_file_actions_destroy(&actions);
			return -1;
		}

		if (HasFlag(options, SubprocessOptions::CombinedOutErr))
		{
			if (0 != posix_spawn_file_actions_adddup2(&actions, STDOUT_FILENO, STDERR_FILENO))
			{
				posix_spawn_file_actions_destroy(&actions);
				return -1;
			}
		}
		else
		{
			// Close the stderr read end
			if (0 != posix_spawn_file_actions_addclose(&actions, stderrfd[0]))
			{
				posix_spawn_file_actions_destroy(&actions);
				return -1;
			}
			// Map the write end to stdout
			if (0 != posix_spawn_file_actions_adddup2(&actions, stderrfd[1], STDERR_FILENO))
			{
				posix_spawn_file_actions_destroy(&actions);
				return -1;
			}
		}

#	ifdef __clang__
#		pragma clang diagnostic push
#		pragma clang diagnostic ignored "-Wcast-qual"
#		pragma clang diagnostic ignored "-Wold-style-cast"
#	endif
		if (HasFlag(options, subprocess_option_search_user_path))
		{
			if (0
			    != posix_spawnp(
			        &child, command[0], &actions, nullptr, (char* const*)command, usedEnvironment))
			{
				posix_spawn_file_actions_destroy(&actions);
				return -1;
			}
		}
		else
		{
			if (0
			    != posix_spawn(
			        &child, command[0], &actions, nullptr, (char* const*)command, usedEnvironment))
			{
				posix_spawn_file_actions_destroy(&actions);
				return -1;
			}
		}
#	ifdef __clang__
#		pragma clang diagnostic pop
#	endif

		// Close the stdin read end
		close(stdinfd[0]);
		// Store the stdin write end
		outProcess->inFile = fdopen(stdinfd[1], "wb");

		// Close the stdout write end
		close(stdoutfd[1]);
		// Store the stdout read end
		outProcess->coutFile = fdopen(stdoutfd[0], "rb");

		if (HasFlag(options, SubprocessOptions::CombinedOutErr))
		{
			outProcess->cerrFile = outProcess->coutFile;
		}
		else
		{
			// Close the stderr write end
			close(stderrfd[1]);
			// Store the stderr read end
			outProcess->cerrFile = fdopen(stderrfd[0], "rb");
		}

		// Store the child's pid
		outProcess->child = child;

		outProcess->alive = true;

		posix_spawn_file_actions_destroy(&actions);
		return 0;
#endif
	}

	i32 WaitProcess(Subprocess* process, int* out_return_code)
	{
#if defined(_MSC_VER)
		const u64 infinite = 0xFFFFFFFF;

		if (process->inFile)
		{
			fclose(process->inFile);
			process->inFile = nullptr;
		}

		if (process->hStdInput)
		{
			::CloseHandle(process->hStdInput);
			process->hStdInput = nullptr;
		}

		::WaitForSingleObject(process->hProcess, infinite);

		if (out_return_code)
		{
			if (!::GetExitCodeProcess(
			        process->hProcess, reinterpret_cast<::DWORD*>(out_return_code)))
			{
				return -1;
			}
		}

		process->alive = false;
		return 0;
#else
		i32 status;

		if (process->inFile)
		{
			fclose(process->inFile);
			process->inFile = nullptr;
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

		if (out_return_code)
		{
			*out_return_code = process->returnStatus;
		}

		return 0;
#endif
	}


	i32 DestroyProcess(Subprocess* process)
	{
		if (process->inFile)
		{
			fclose(process->inFile);
			process->inFile = nullptr;
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
			}

			if (process->hEventOutput)
			{
				CloseHandle(process->hEventOutput);
			}

			if (process->hEventError)
			{
				CloseHandle(process->hEventError);
			}
		}
#endif

		return 0;
	}

	i32 TerminateProcess(Subprocess* process)
	{
#if defined(_MSC_VER)
		u32 killed_process_exit_code;
		i32 success_terminate;
		i32 windows_call_result;

		killed_process_exit_code = 99;
		windows_call_result      = ::TerminateProcess(process->hProcess, killed_process_exit_code);
		success_terminate        = (windows_call_result == 0) ? 1 : 0;
		return success_terminate;
#else
		i32 result;
		result = kill(process->child, 9);
		return result;
#endif
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
		        reinterpret_cast<LPOVERLAPPED>(&overlapped)))
		{
			const u64 errorIoPending = 997;
			u64 error                = GetLastError();

			// Means we've got an async read!
			if (error == errorIoPending)
			{
				if (!::GetOverlappedResult(handle, reinterpret_cast<LPOVERLAPPED>(&overlapped),
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
		        reinterpret_cast<LPOVERLAPPED>(&overlapped)))
		{
			const u64 errorIoPending = 997;
			u64 error                = GetLastError();

			// Means we've got an async read!
			if (error == errorIoPending)
			{
				if (!::GetOverlappedResult(handle, reinterpret_cast<LPOVERLAPPED>(&overlapped),
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
		return process->inFile;
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
}    // namespace p::core
