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
		struct Scope
		{
			u32 id             = 0;
			u32 size           = 0;
			yyjson_val* parent = nullptr;
		};
		yyjson_doc* doc  = nullptr;
		yyjson_val* root = nullptr;
		yyjson_val* current = nullptr;
		TArray<Scope> scopeStack;


	public:
		/**
		 * Constructs a JsonParser.
		 * @param customParser will override the global parser. Reusing parsers can improve
		 * performance, but they are not thread-safe. While multithreading, use a parser for each
		 * thread.
		 */
		CORE_API JsonFormatReader(const String& data);
		CORE_API ~JsonFormatReader();

		CORE_API void BeginObject();
		CORE_API void BeginArray(u32& size);

		CORE_API bool EnterNext(StringView name);
		CORE_API bool EnterNext();
		CORE_API void Leave();

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


		CORE_API bool IsObject() const;
		CORE_API bool IsArray() const;
		CORE_API bool IsValid() const
		{
			return root != nullptr;
		}

	private:
		u32 InternalBegin();
		bool FindNextKey(u32 firstId, yyjson_val* firstKey, StringView name, u32& outIndex,
		    yyjson_val*& outValue);
		Scope& GetScope();
		void PushScope(yyjson_val* newScope);
		void PopScope();
	};


	struct JsonFormatWriter : public TFormatWriter<Format_Json>
	{
	private:
	public:
		CORE_API bool EnterNext(StringView name);
		CORE_API bool EnterNext();
		CORE_API void Leave();

		CORE_API void BeginObject();
		CORE_API void BeginArray(u32& size);

		CORE_API void Write(WriteContext& ct, const bool& val);
		CORE_API void Write(WriteContext& ct, const u8& val);
		CORE_API void Write(WriteContext& ct, const i32& val);
		CORE_API void Write(WriteContext& ct, const u32& val);
		CORE_API void Write(WriteContext& ct, const i64& val);
		CORE_API void Write(WriteContext& ct, const u64& val);
		CORE_API void Write(WriteContext& ct, const float& val);
		CORE_API void Write(WriteContext& ct, const double& val);
		CORE_API void Write(WriteContext& ct, const StringView& val);
		CORE_API void Write(WriteContext& ct, const String& val);

		CORE_API bool IsObject() const;
		CORE_API bool IsArray() const;
		CORE_API bool IsValid() const
		{
			return false;    // root != nullptr;
		}
	};


	template <>
	struct FormatBind<Format_Json>
	{
		using Reader                    = JsonFormatReader;
		using Writer                    = JsonFormatWriter;
	};
}    // namespace Rift::Serl
