// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Serialization/Formats/TFormat.h"
#include "Serialization/Json.h"
#include "Serialization/ReadContext.h"
#include "Serialization/WriteContext.h"


struct yyjson_doc;
struct yyjson_mut_doc;
struct yyjson_val;
struct yyjson_mut_val;


namespace Rift::Serl
{
	struct CORE_API JsonFormatReader : public TFormatReader<Format::Json>
	{
	private:
		struct Scope
		{
			u32 id             = 0;
			u32 size           = 0;
			yyjson_val* parent = nullptr;
		};
		yyjson_doc* doc  = nullptr;
		yyjson_val* root    = nullptr;
		yyjson_val* current = nullptr;
		TArray<Scope> scopeStack;


	public:
		/**
		 * Constructs a JsonParser.
		 * @param customParser will override the global parser. Reusing parsers can improve
		 * performance, but they are not thread-safe. While multithreading, use a parser for each
		 * thread.
		 */
		JsonFormatReader(const String& data);
		~JsonFormatReader();

		void BeginObject();
		void BeginArray(u32& size);

		bool EnterNext(StringView name);
		bool EnterNext();
		void Leave();

		void Read(bool& val);
		void Read(u8& val);
		void Read(i32& val);
		void Read(u32& val);
		void Read(i64& val);
		void Read(u64& val);
		void Read(float& val);
		void Read(double& val);
		void Read(StringView& val);

		bool IsObject() const;
		bool IsArray() const;
		bool IsValid() const
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


	struct CORE_API JsonFormatWriter : public TFormatWriter<Format::Json>
	{
	private:
		struct Scope
		{
			StringView key;
			yyjson_mut_val* parent = nullptr;
		};
		yyjson_mut_doc* doc      = nullptr;
		yyjson_mut_val* current  = nullptr;
		TArray<Scope> scopeStack;
		bool open = true;


	public:
		JsonFormatWriter();
		~JsonFormatWriter();

		bool EnterNext(StringView name);
		bool EnterNext();
		void Leave();

		void BeginObject();
		void BeginArray(u32& size);

		void Write(bool val);
		void Write(u8 val);
		void Write(i32 val);
		void Write(u32 val);
		void Write(i64 val);
		void Write(u64 val);
		void Write(float val);
		void Write(double val);
		void Write(StringView val);

		bool IsValid() const
		{
			return doc != nullptr;
		}

		void Close();

		StringView ToString(bool pretty = true, bool ensureClosed = true);

	private:
		Scope& GetScope();
		void PushScope(StringView key);
		void PopScope();
	};


	template <>
	struct FormatBind<Format::Json>
	{
		using Reader                    = JsonFormatReader;
		using Writer                    = JsonFormatWriter;
	};
}    // namespace Rift::Serl
