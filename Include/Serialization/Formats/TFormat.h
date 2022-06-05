// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Serialization/Contexts/ReadContext.h"
#include "Serialization/Contexts/WriteContext.h"
#include "Serialization/Formats/IFormat.h"


namespace pipe::serl
{
	template<Format format>
	struct TFormatReader : public IFormatReader
	{
	private:
		ReadContext context{};


	public:
		TFormatReader()
		{
			context.flags  = Flags_None;
			context.format = format;
			context.reader = this;
		}

		ReadContext& GetContext()
		{
			return context;
		}

		operator ReadContext&()
		{
			return GetContext();
		}
	};


	template<Format format>
	struct TFormatWriter : public IFormatWriter
	{
	private:
		WriteContext context{};


	public:
		TFormatWriter()
		{
			context.flags  = Flags_None;
			context.format = format;
			context.writer = this;
		}

		WriteContext& GetContext()
		{
			return context;
		}

		operator WriteContext&()
		{
			return GetContext();
		}
	};
}    // namespace pipe::serl
