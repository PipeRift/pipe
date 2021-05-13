// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Serialization/Format_Json.h"


namespace Rift::Serl
{
	static JsonParser globalParser{};


	FormatReader_Json::FormatReader_Json(const String& data, JsonParser* customParser)
	    : parser(customParser ? *customParser : globalParser)
	{
		// parser.parser.iterate(data);
	}
}    // namespace Rift::Serl
