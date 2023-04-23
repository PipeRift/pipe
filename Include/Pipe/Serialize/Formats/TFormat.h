// Copyright 2015-2023 Piperift - All rights reserved
#pragma once

#include "Pipe/Serialize/Formats/IFormat.h"
#include "Pipe/Serialize/Reader.h"
#include "Pipe/Serialize/Writer.h"


namespace p
{
	template<SerializeFormat format>
	struct TFormatReader : public IFormatReader
	{
	private:
		Reader reader{};


	public:
		TFormatReader()
		{
			reader.format       = format;
			reader.formatReader = this;
		}

		Reader& GetReader()
		{
			return reader;
		}

		operator Reader&()
		{
			return GetReader();
		}
	};


	template<SerializeFormat format>
	struct TFormatWriter : public IFormatWriter
	{
	private:
		Writer writer{};


	public:
		TFormatWriter()
		{
			writer.format       = format;
			writer.formatWriter = this;
		}

		Writer& GetWriter()
		{
			return writer;
		}

		operator Writer&()
		{
			return GetWriter();
		}
	};
}    // namespace p
