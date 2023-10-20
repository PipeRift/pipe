// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Core/Checks.h"

#include "Pipe/Core/Log.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/StringView.h"


namespace p::internal
{
	void FailedCheckError(const AnsiChar* expr, const AnsiChar* file, u32 line, const char* inText)
	{
		p::String text{inText};
		if (text.empty())
		{
			p::Strings::FormatTo(text, "Failed check \"{}\" at {}:{}", p::StringView{expr},
			    p::StringView{file}, line);
		}
		else
		{
			p::Strings::FormatTo(text, "\n(Failed check \"{}\" at {}:{})", p::StringView{expr},
			    p::StringView{file}, line);
		}
		Error(text);
	}
}    // namespace p::internal
