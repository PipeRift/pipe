// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Span.h"
#include "Pipe/Core/String.h"
#include "Pipe/Serialize/Formats/TFormat.h"


namespace p
{
	struct BinaryFormatReader : public TFormatReader<SerializeFormat::Binary>
	{
	private:
		TSpan<p::u8> data;
		p::u8* pos = nullptr;

	public:
		PIPE_API BinaryFormatReader(TSpan<p::u8> data);
		PIPE_API ~BinaryFormatReader();

		PIPE_API void BeginObject() override;
		PIPE_API void BeginArray(u32& size) override;

		PIPE_API bool EnterNext(StringView name) override;
		PIPE_API bool EnterNext() override;
		PIPE_API void Leave() override;

		PIPE_API void Read(bool& val) override;
		PIPE_API void Read(i8& val) override;
		PIPE_API void Read(u8& val) override;
		PIPE_API void Read(i16& val) override;
		PIPE_API void Read(u16& val) override;
		PIPE_API void Read(i32& val) override;
		PIPE_API void Read(u32& val) override;
		PIPE_API void Read(i64& val) override;
		PIPE_API void Read(u64& val) override;
		PIPE_API void Read(float& val) override;
		PIPE_API void Read(double& val) override;
		PIPE_API void Read(StringView& val) override;

		PIPE_API bool IsObject() const override;
		PIPE_API bool IsArray() const override;
		PIPE_API bool IsValid() const override
		{
			return data.Data() && !data.IsEmpty();
		}
	};

	struct BinaryFormatWriter : public TFormatWriter<SerializeFormat::Binary>
	{
	private:
		p::u8* data     = nullptr;
		p::i32 size     = 0;
		p::i32 capacity = 0;


	public:
		PIPE_API BinaryFormatWriter();
		PIPE_API ~BinaryFormatWriter();

		// BEGIN Writer Interface
		PIPE_API bool EnterNext(StringView name) override {}    // Nothing to do
		PIPE_API bool EnterNext() override {}                   // Nothing to do
		PIPE_API void Leave() override {}                       // Nothing to do
		PIPE_API void BeginObject() override {}                 // Nothing to do
		PIPE_API void BeginArray(u32& size) override {}         // Nothing to do
		PIPE_API void Write(bool val) override;
		PIPE_API void Write(i8 val) override;
		PIPE_API void Write(u8 val) override;
		PIPE_API void Write(i16 val) override;
		PIPE_API void Write(u16 val) override;
		PIPE_API void Write(i32 val) override;
		PIPE_API void Write(u32 val) override;
		PIPE_API void Write(i64 val) override;
		PIPE_API void Write(u64 val) override;
		PIPE_API void Write(float val) override;
		PIPE_API void Write(double val) override;
		PIPE_API void Write(StringView val) override;
		PIPE_API bool IsValid() const override
		{
			return data != nullptr;
		}
		// END Writer Interface

		PIPE_API TSpan<p::u8> GetData();

	private:
		void PreAlloc(p::u32 offset);
	};


	template<>
	struct FormatBind<SerializeFormat::Binary>
	{
		using Reader = BinaryFormatReader;
		using Writer = BinaryFormatWriter;
	};
}    // namespace p