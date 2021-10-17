// Copyright 2015-2021 Piperift - All rights reserved
#pragma once

#include "Serialization/Formats/TFormat.h"
#include "Strings/String.h"


struct yyjson_doc;
struct yyjson_mut_doc;
struct yyjson_val;
struct yyjson_mut_val;


namespace Rift::Serl
{
	struct CORE_API JsonFormatReader : public TFormatReader<Format::Json>
	{
		enum class ReadErrorCode : u32
		{
			InvalidParameter    = 1,
			MemoryAllocation    = 2,
			EmptyContent        = 3,
			UnexpectedContent   = 4,
			UnexpectedEnd       = 5,
			UnexpectedCharacter = 6,
			JsonStructure       = 7,
			InvalidComment      = 8,
			InvalidNumber       = 9,
			InvalidString       = 10,
			ErrorLiteral        = 11,
			FileOpen            = 12,
			FileRead            = 13
		};

		struct ReadError
		{
			ReadErrorCode code;
			/** Short error message (NULL for success). */
			const char* msg;
			/** Error byte position for input data (0 for success). */
			sizet pos;
		};

	private:
		struct Scope
		{
			u32 id             = 0;
			u32 size           = 0;
			yyjson_val* parent = nullptr;
		};
		String insituBuffer;
		yyjson_doc* doc     = nullptr;
		yyjson_val* root    = nullptr;
		yyjson_val* current = nullptr;
		TArray<Scope> scopeStack;
		ReadError error;


	public:
		/**
		 * Configures a JsonFormatReader to read from an string buffer
		 * @param data containing the constant json string
		 * @see JsonFormatReader(String& data, bool insitu = true) for optional insitu reading
		 */
		explicit JsonFormatReader(StringView data);
		/**
		 * Configures a JsonFormatReader to read from an string buffer
		 * This contructor might MODIFY the buffer if needed to improve reading speed
		 * slightly.
		 * @param data containing the MUTABLE json string
		 */
		explicit JsonFormatReader(String& data);
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
		const ReadError& GetError() const
		{
			return error;
		}

	private:
		void InternalInit(char* data, sizet size, bool insitu);

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
		yyjson_mut_doc* doc     = nullptr;
		yyjson_mut_val* current = nullptr;
		StringBuffer stringBuffer;    // Stores temporal string keys and values
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
		using Reader = JsonFormatReader;
		using Writer = JsonFormatWriter;
	};
}    // namespace Rift::Serl
