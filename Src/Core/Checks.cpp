// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "Pipe/Core/Checks.h"

#include "Pipe/Core/Log.h"
#include "PipeStrings.h"
#include "Pipe/Core/StringView.h"


namespace p::details
{
	void FailedCheckError(const AnsiChar* expr, const AnsiChar* file, u32 line, const char* inText)
	{
		p::String text{inText};
		if (text.empty())
		{
			FormatTo(text, "Failed check \"{}\" at {}:{}", p::StringView{expr},
			    p::StringView{file}, line);
		}
		else
		{
			FormatTo(text, "\n(Failed check \"{}\" at {}:{})", p::StringView{expr},
			    p::StringView{file}, line);
		}
		Error(text);
	}
}    // namespace p::details
