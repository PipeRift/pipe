// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Core/Log.h"

#include "Pipe/Files/Files.h"
#include "Pipe/Files/Paths.h"
#include "Pipe/Memory/OwnPtr.h"
#include "PipeTime.h"

#include <iostream>

namespace p
{
	void InitLog(StringView logPath) {}

	void ShutdownLog() {}

	void Info(StringView msg)
	{
		String text;
		auto now = DateTime::Now();
		now.ToString("[%Y/%m/%d %H:%M:%S]", text);
		Strings::FormatTo(text, "[Info] {}", msg);
		std::cout << text << std::endl;
	}

	void Warning(StringView msg)
	{
		String text;
		auto now = DateTime::Now();
		now.ToString("[%Y/%m/%d %H:%M:%S]", text);
		Strings::FormatTo(text, "[Warning] {}", msg);
		std::cout << text << std::endl;
	}

	void Error(StringView msg)
	{
		String text;
		auto now = DateTime::Now();
		now.ToString("[%Y/%m/%d %H:%M:%S]", text);
		Strings::FormatTo(text, "[Error] {}", msg);
		std::cout << text << std::endl;
	}
}    // namespace p
