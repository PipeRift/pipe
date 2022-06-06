// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Serialization/Formats/IFormat.h"
#include "Serialization/Reader.h"
#include "Serialization/Writer.h"



namespace p
{
	template<SerializeFormat format>
	struct TFormatReader : public IFormatReader
	{
	private:
		Reader context{};


	public:
		TFormatReader()
		{
			context.format = format;
			context.reader = this;
		}

		Reader& GetContext()
		{
			return context;
		}

		operator Reader&()
		{
			return GetContext();
		}
	};


	template<SerializeFormat format>
	struct TFormatWriter : public IFormatWriter
	{
	private:
		Writer context{};


	public:
		TFormatWriter()
		{
			context.format = format;
			context.writer = this;
		}

		Writer& GetContext()
		{
			return context;
		}

		operator Writer&()
		{
			return GetContext();
		}
	};
}    // namespace p
