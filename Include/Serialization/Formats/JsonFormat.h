// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Serialization/Formats/TFormat.h"
#include "Strings/String.h"


struct yyjson_doc;
struct yyjson_mut_doc;
struct yyjson_val;
struct yyjson_mut_val;


namespace Pipe::Serl
{
	struct JsonFormatReader : public TFormatReader<Format::Json>
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
		CORE_API explicit JsonFormatReader(StringView data);
		/**
		 * Configures a JsonFormatReader to read from an string buffer
		 * This contructor might MODIFY the buffer if needed to improve reading speed
		 * slightly.
		 * @param data containing the MUTABLE json string
		 */
		CORE_API explicit JsonFormatReader(String& data);
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

		CORE_API bool IsObject() const;
		CORE_API bool IsArray() const;
		CORE_API bool IsValid() const
		{
			return root != nullptr;
		}
		CORE_API const ReadError& GetError() const
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


	struct JsonFormatWriter : public TFormatWriter<Format::Json>
	{
	private:
		struct Scope
		{
			yyjson_mut_val* key    = nullptr;
			yyjson_mut_val* parent = nullptr;
		};
		yyjson_mut_doc* doc     = nullptr;
		yyjson_mut_val* current = nullptr;
		TArray<Scope> scopeStack;
		bool open = true;


	public:
		CORE_API JsonFormatWriter();
		CORE_API ~JsonFormatWriter();

		CORE_API bool EnterNext(StringView name);
		CORE_API bool EnterNext();
		CORE_API void Leave();

		CORE_API void BeginObject();
		CORE_API void BeginArray(u32& size);

		CORE_API void Write(bool val);
		CORE_API void Write(u8 val);
		CORE_API void Write(i32 val);
		CORE_API void Write(u32 val);
		CORE_API void Write(i64 val);
		CORE_API void Write(u64 val);
		CORE_API void Write(float val);
		CORE_API void Write(double val);
		CORE_API void Write(StringView val);

		CORE_API bool IsValid() const
		{
			return doc != nullptr;
		}

		CORE_API void Close();

		CORE_API StringView ToString(bool pretty = true, bool ensureClosed = true);

	private:
		Scope& GetScope();
		void PushScope(StringView key);
		void PopScope();
	};


	template<>
	struct FormatBind<Format::Json>
	{
		using Reader = JsonFormatReader;
		using Writer = JsonFormatWriter;
	};
}    // namespace Pipe::Serl
