// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Core/Log.h"

#include "Pipe/Files/Files.h"
#include "Pipe/Files/Paths.h"
#include "Pipe/Memory/OwnPtr.h"
#include "PipeTime.h"

#include <iostream>

namespace p
{
	// clang-format off
	const Logger defaultLogger = Logger{
		.infoCallback = [](StringView msg) {
			String text;
			auto now = DateTime::Now();
			now.ToString("[%Y/%m/%d %H:%M:%S]", text);
			Strings::FormatTo(text, "[Info] {}\n", msg);
			std::cout << text;
		},
		.warningCallback = [](StringView msg) {
			String text;
			auto now = DateTime::Now();
			now.ToString("[%Y/%m/%d %H:%M:%S]", text);
			Strings::FormatTo(text, "[Warning] {}\n", msg);
			std::cout << text;
		},
		.errorCallback = [](StringView msg) {
			String text;
			auto now = DateTime::Now();
			now.ToString("[%Y/%m/%d %H:%M:%S]", text);
			Strings::FormatTo(text, "[Error] {}\n", msg);
			std::cout << text;
		}
	};
	// clang-format on

	const Logger* globalLogger = nullptr;

	void InitLog(Logger* logger)
	{
		globalLogger = logger ? logger : &defaultLogger;
	}

	void ShutdownLog()
	{
		globalLogger = nullptr;
	}

	void Info(StringView msg)
	{
		if (globalLogger)
		{
			globalLogger->infoCallback(msg);
		}
	}

	void Warning(StringView msg)
	{
		if (globalLogger)
		{
			globalLogger->warningCallback(msg);
		}
	}

	void Error(StringView msg)
	{
		if (globalLogger)
		{
			globalLogger->errorCallback(msg);
		}
	}
}    // namespace p
