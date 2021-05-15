// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Serialization/Formats/JsonFormat.h"
#include "Strings/String.h"

#include <yyjson.h>


namespace Rift::Serl
{
	JsonFormatReader::JsonFormatReader(const String& data)
	{
		// TODO: Support json from file (yyjson_read_file)
		doc  = yyjson_read(data.data(), data.length(), 0);
		root = yyjson_doc_get_root(doc);
	}

	JsonFormatReader::~JsonFormatReader()
	{
		yyjson_doc_free(doc);
	}
}    // namespace Rift::Serl
