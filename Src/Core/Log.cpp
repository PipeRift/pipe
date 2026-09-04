// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Core/Log.h"

#include <iostream>

namespace p
{
	// clang-format off
	const Logger defaultLogger = Logger{
	.infoCallback = [](StringView msg)
	{
		std::cout << msg << '\n';
	},
	.warningCallback = [](StringView msg)
	{
		std::cout << msg << '\n';
	},
	.errorCallback = [](StringView msg)
	{
		std::cerr << msg << '\n';
	}};
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
