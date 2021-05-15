// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Serialization/Formats/IFormat.h"
#include "Serialization/ReadContext.h"


namespace Rift::Serl
{
	template <Format format>
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


	template <Format format>
	struct TFormatWriter : public IFormatWriter
	{};
}    // namespace Rift::Serl
