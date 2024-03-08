// Copyright 2015-2023 Piperift - All rights reserved
// Modified from https://github.com/sheredom/subprocess.h
#pragma once

#include "Pipe/Core/EnumFlags.h"
#include "Pipe/Core/Optional.h"
#include "Pipe/Export.h"
#include "PipeArrays.h"

#include <stdio.h>
#include <string.h>


#if !defined(_MSC_VER)
	#include <signal.h>
	#include <spawn.h>
	#include <stdlib.h>
	#include <sys/types.h>
	#include <sys/wait.h>
	#include <unistd.h>
#endif


namespace p
{
	enum class PIPE_API SubprocessOptions
	{
		None = 0,

		// stdout and stderr are the same FILE.
		CombinedOutErr = 1 << 0,

		// The child process should inherit the environment variables of the parent.
		InheritEnvironment = 1 << 1,

		// Enable asynchronous reading of stdout/stderr before it has completed.
		EnableAsync = 1 << 2,

		// Enable the child process to be spawned with no window visible if supported
		// by the platform.
		NoWindow = 1 << 3,

		// If set, the process will terminate when the instance is destroyed
		TerminateIfDestroyed = 1 << 4,

		// Search for program names in the PATH variable. Always enabled on Windows.
		// Note: this will **not** search for paths in any provided custom environment
		// and instead uses the PATH of the spawning process.
		SearchUserPath = InheritEnvironment | NoWindow
	};
	P_DEFINE_FLAG_OPERATORS(SubprocessOptions)


#ifdef __clang__
	#pragma clang diagnostic push
	#pragma clang diagnostic ignored "-Wpadded"
#endif
	struct PIPE_API Subprocess
	{
		FILE* cinFile  = nullptr;
		FILE* coutFile = nullptr;
		FILE* cerrFile = nullptr;

#if defined(_MSC_VER)
		void* hProcess     = nullptr;
		void* hStdInput    = nullptr;
		void* hEventOutput = nullptr;
		void* hEventError  = nullptr;
#else
		pid_t child      = 0;
		i32 returnStatus = 0;
#endif

		SubprocessOptions options = SubprocessOptions::None;
		bool alive                = false;


		Subprocess() = default;
		Subprocess(Subprocess&& other) noexcept;
		~Subprocess();
	};
#ifdef __clang__
	#pragma clang diagnostic pop
#endif


	/**
	 * @brief Create a process (extended create).
	 * @param command An array of strings for the command line to execute for
	 * this process. The last element must be NULL to signify the end of the array.
	 * The memory backing this parameter only needs to persist until this function
	 * returns.
	 * @param options A bit field of subprocess_option_e's to pass.
	 * @param environment An optional array of strings for the environment to use
	 * for a child process (each element of the form FOO=BAR). The last element
	 * must be NULL to signify the end of the array.
	 * @param outProcess The newly created process.
	 * @return On success zero is returned.
	 *
	 * If `options` contains `subprocess_option_inherit_environment`, then
	 * `environment` must be NULL.
	 */
	PIPE_API TOptional<Subprocess> RunProcessEx(TView<const char* const> command,
	    TView<const char* const> environment, SubprocessOptions options = SubprocessOptions::None);

	/**
	 * @brief Create a process.
	 * @param command An array of strings for the command line to execute for
	 * this process. The last element must be NULL to signify the end of the array.
	 * The memory backing this parameter only needs to persist until this function
	 * returns.
	 * @param options A bit field of subprocess_option_e's to pass.
	 * @param outProcess The newly created process.
	 * @return On success zero is returned.
	 */
	inline PIPE_API TOptional<Subprocess> RunProcess(
	    TView<const char* const> command, SubprocessOptions options = SubprocessOptions::None)
	{
		return RunProcessEx(command, {}, options);
	}

	/**
	 * @brief Wait for a process to finish execution.
	 * @param process The process to wait for.
	 * @param out_return_code The return code of the returned process (can be
	 * NULL).
	 * @return On success zero is returned.
	 *
	 * Joining a process will close the stdin pipe to the process.
	 */
	PIPE_API i32 WaitProcess(Subprocess* process, i32* outReturnCode);

	/**
	 * @brief Destroy a previously created process.
	 * @param process The process to destroy.
	 * @return On success zero is returned.
	 *
	 * If the process to be destroyed had not finished execution, it may out live
	 * the parent process.
	 */
	PIPE_API i32 DestroyProcess(Subprocess* process);

	/**
	 * @brief Terminate a previously created process.
	 * @param process The process to terminate.
	 * @return On success zero is returned.
	 *
	 * If the process to be destroyed had not finished execution, it will be
	 * terminated (i.e killed).
	 */
	PIPE_API i32 TerminateProcess(Subprocess* process);

	/**
	 * @brief Read the standard output from the child process.
	 * @param process The process to read from.
	 * @param buffer The buffer to read into.
	 * @param size The maximum number of bytes to read.
	 * @return The number of bytes actually read into buffer. Can only be 0 if the
	 * process has complete.
	 *
	 * The only safe way to read from the standard output of a process during it's
	 * execution is to use the `SubprocessOptions::EnableAsync` option in
	 * conjuction with this method.
	 */
	PIPE_API u32 ReadProcessCout(Subprocess* process, char* buffer, u32 size);

	/**
	 * @brief Read the standard error from the child process.
	 * @param process The process to read from.
	 * @param buffer The buffer to read into.
	 * @param size The maximum number of bytes to read.
	 * @return The number of bytes actually read into buffer. Can only be 0 if the
	 * process has complete.
	 *
	 * The only safe way to read from the standard error of a process during it's
	 * execution is to use the `SubprocessOptions::EnableAsync` option in
	 * conjuction with this method.
	 */
	PIPE_API u32 ReadProcessCerr(Subprocess* process, char* buffer, u32 size);

	/**
	 * @brief Returns if the subprocess is currently still alive and executing.
	 * @param process The process to check.
	 * @return If the process is still alive non-zero is returned.
	 */
	PIPE_API bool IsAlive(Subprocess* process);

	/**
	 * @brief Get the standard input file for a process.
	 * @param process The process to query.
	 * @return The file for standard input of the process.
	 *
	 * The file returned can be written to by the parent process to feed data to
	 * the standard input of the process.
	 */
	PIPE_API FILE* GetProcessCin(const Subprocess* process);

	/**
	 * @brief Get the standard output file for a process.
	 * @param process The process to query.
	 * @return The file for standard output of the process.
	 *
	 * The file returned can be read from by the parent process to read data from
	 * the standard output of the child process.
	 */
	PIPE_API FILE* GetProcessCout(const Subprocess* process);

	/**
	 * @brief Get the standard error file for a process.
	 * @param process The process to query.
	 * @return The file for standard error of the process.
	 *
	 * The file returned can be read from by the parent process to read data from
	 * the standard error of the child process.
	 *
	 * If the process was created with the SubprocessOptions::CombinedOutErr
	 * option bit set, this function will return NULL, and the GetProcessCout
	 * function should be used for both the standard output and error combined.
	 */
	PIPE_API FILE* GetProcessCerr(const Subprocess* process);
}    // namespace p
