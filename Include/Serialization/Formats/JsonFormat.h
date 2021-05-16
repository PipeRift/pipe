// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Serialization/Formats/TFormat.h"
#include "Serialization/Json.h"
#include "Serialization/ReadContext.h"
#include "Serialization/WriteContext.h"


struct yyjson_doc;
struct yyjson_val;


namespace Rift::Serl
{
	struct JsonFormatReader : public TFormatReader<Format_Json>
	{
	private:
		yyjson_doc* doc  = nullptr;
		yyjson_val* root = nullptr;
		yyjson_val* currentScope = nullptr;
		TArray<yyjson_val*> scopeStack;
 

	public:
		/**
		 * Constructs a JsonParser.
		 * @param customParser will override the global parser. Reusing parsers can improve
		 * performance, but they are not thread-safe. While multithreading, use a parser for each
		 * thread.
		 */
		CORE_API JsonFormatReader(const String& data);
		CORE_API ~JsonFormatReader();

		CORE_API bool EnterScope(StringView name);
		CORE_API bool EnterScope(u32 index);
		CORE_API void LeaveScope();

		CORE_API void IterateObject(TFunction<void()> callback);
		CORE_API void IterateObject(TFunction<void(const char*)> callback);
		CORE_API bool IsObject() const;
		CORE_API sizet GetObjectSize() const;

		CORE_API void IterateArray(TFunction<void()> callback);
		CORE_API void IterateArray(TFunction<void(u32)> callback);
		CORE_API bool IsArray() const;
		CORE_API sizet GetArraySize() const;

		CORE_API void Read(bool& val);
		CORE_API void Read(u8& val);
		CORE_API void Read(i32& val);
		CORE_API void Read(u32& val);
		CORE_API void Read(i64& val);
		CORE_API void Read(u64& val);
		CORE_API void Read(float& val);
		CORE_API void Read(double& val);
		CORE_API void Read(StringView& val);
		CORE_API void Read(String& val);

		CORE_API bool IsValid() const
		{
			return root != nullptr;
		}
		CORE_API sizet GetSize() const
		{
			return IsObject()? GetObjectSize() : GetArraySize();
		}
	};


	struct JsonFormatWriter : public TFormatWriter<Format_Json>
	{};


	template <>
	struct FormatBind<Format_Json>
	{
		using Reader                    = JsonFormatReader;
		using Writer                    = JsonFormatWriter;
	};
}    // namespace Rift::Serl
