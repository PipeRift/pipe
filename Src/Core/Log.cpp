// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Core/Log.h"

#include "Pipe/Core/Profiler.h"
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
#	include <spdlog/details/windows_include.h>
#endif


namespace p::Log
{
	TOwnPtr<spdlog::logger> generalLogger;
	TOwnPtr<spdlog::logger> errorLogger;


#if P_ENABLE_PROFILER
	template<typename Mutex>
	class ProfilerSink : public spdlog::sinks::base_sink<Mutex>
	{
		using Super = spdlog::sinks::base_sink<Mutex>;

	public:
		ProfilerSink() : Super()
		{
			Super::set_pattern("%^[%t][%l]%$ %v");
		}

	protected:
		void sink_it_(const spdlog::details::log_msg& msg) override
		{
			// log_msg is a struct containing the log entry info like level,
			// timestamp, thread id etc. msg.raw contains pre formatted log

			// If needed (very likely but not mandatory), the sink formats the
			// message before sending it to its final destination:
			spdlog::memory_buf_t formatted;
			Super::formatter_->format(msg, formatted);

			TracyMessage(formatted.data(), formatted.size());    // Send to profiler
		}

		void flush_() override {}
	};
	using ProfilerSink_mt = ProfilerSink<std::mutex>;
	using ProfilerSink_st = ProfilerSink<spdlog::details::null_mutex>;
#endif


	void Init(Path logFile)
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
		// Profiler
#if P_ENABLE_PROFILER
		sinks.Add(std::make_shared<ProfilerSink_mt>());
#endif

		// File
		if (!logFile.empty())
		{
			Path logFolder = logFile;
			if (files::IsFile(logFile))
			{
				logFolder.remove_filename();
			}
			else
			{
				logFile /= "log.txt";
			}
			files::CreateFolder(logFolder, true);

			sinks.Add(std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
			    ToString(logFile).c_str(), 1048576 * 5, 3));
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

	void Shutdown()
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
}    // namespace p::Log
