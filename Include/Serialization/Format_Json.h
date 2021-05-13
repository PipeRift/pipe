// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Serialization/IFormat.h"
#include "Serialization/Json.h"

#include <simdjson.h>


namespace Rift::Serl
{
	struct JsonParser
	{
		simdjson::dom::parser parser{};
	};

	struct FormatReader_Json : public IFormatReader
	{
		JsonParser& parser;


		/**
		 * Constructs a JsonParser.
		 * @param customParser will override the global parser. Reusing parsers can improve
		 * performance, but they are not thread-safe. While multithreading, use a parser for each
		 * thread.
		 */
		CORE_API FormatReader_Json(const String& data, JsonParser* customParser = nullptr);

		CORE_API void ReadChild(StringView name);

		CORE_API void Read(StringView name, bool& val) {}

		CORE_API void Read(StringView name, u8& val) {}

		CORE_API void Read(StringView name, i32& val) {}

		CORE_API void Read(StringView name, u32& val) {}

		CORE_API void Read(StringView name, float& val) {}

		CORE_API void Read(StringView name, String& val) {}

		CORE_API void Read(StringView name, Json& val) {}

		bool IsArray()
		{
			return false;
		}
	};

	template <>
	struct FormatBind<Format_Json>
	{
		using Reader = FormatReader_Json;
	};
}    // namespace Rift::Serl
