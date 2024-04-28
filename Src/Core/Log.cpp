// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Core/Log.h"

#include "Pipe/Files/Files.h"
#include "Pipe/Files/Paths.h"
#include "Pipe/Memory/OwnPtr.h"

#include <spdlog/details/log_msg.h>
#include <spdlog/details/null_mutex.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/base_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>


#if P_PLATFORM_WINDOWS
	#include <spdlog/details/windows_include.h>
#endif


namespace p
{
	TOwnPtr<spdlog::logger> generalLogger;
	TOwnPtr<spdlog::logger> errorLogger;


	void InitLog(StringView logPath)
	{
		TArray<spdlog::sink_ptr> sinks;
		sinks.Reserve(3);

		// Sinks /////////////////////////////////////////
		// Console
		auto cliSink    = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto cliErrSink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
		cliSink->set_pattern("%^%v%$");
		cliErrSink->set_pattern("%^[%l] %v%$");
#if P_PLATFORM_WINDOWS
		cliSink->set_color(
		    spdlog::level::info, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		cliErrSink->set_color(spdlog::level::warn, FOREGROUND_RED | FOREGROUND_GREEN);
#else
		cliSink->set_color(spdlog::level::info, cliSink->white);
		cliErrSink->set_color(spdlog::level::warn, cliSink->yellow);
#endif

		// File
		if (!logPath.empty())
		{
			String logFile{logPath};
			if (!IsFile(logFile))
			{
				AppendToPath(logFile, "log.log");
			}
			CreateFolder(GetParentPath(logFile), true);

			sinks.Add(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
			    logFile.c_str(), 1048576 * 5, 3));
		}

		// Loggers /////////////////////////////////////////
		sinks.Add(cliSink);
		generalLogger = MakeOwned<spdlog::logger>("Log", sinks.begin(), sinks.end());
		generalLogger->set_pattern("%^[%D %T][%l]%$ %v");

		sinks.RemoveLast();
		sinks.Add(cliErrSink);
		errorLogger = MakeOwned<spdlog::logger>("Log", sinks.begin(), sinks.end());
		errorLogger->set_pattern("%^[%D %T][%t][%l]%$ %v");
	}

	void ShutdownLog()
	{
		generalLogger.Delete();
		errorLogger.Delete();
	}

	void Info(StringView msg)
	{
		generalLogger->info(msg);
	}

	void Warning(StringView msg)
	{
		errorLogger->warn(msg);
	}

	void Error(StringView msg)
	{
		errorLogger->error(msg);
	}
}    // namespace p
