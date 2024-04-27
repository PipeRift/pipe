// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Serialize/Formats/IFormat.h"
#include "PipeArraysFwd.h"


namespace p
{
	struct BinaryFormatReader : public IFormatReader
	{
	private:
		TView<u8> data;
		u8* pointer = nullptr;

	public:
		PIPE_API BinaryFormatReader(TView<u8> data);
		PIPE_API ~BinaryFormatReader();

		PIPE_API void BeginObject() override {}    // Nothing to do
		PIPE_API void BeginArray(u32& size) override;

		PIPE_API bool EnterNext(StringView name) override;    // Nothing to do
		PIPE_API bool EnterNext() override;                   // Nothing to do
		PIPE_API void Leave() override {}                     // Nothing to do

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
		PIPE_API bool IsValid() const override;
	};

	struct BinaryFormatWriter : public IFormatWriter
	{
	private:
		Arena& arena;
		u8* data     = nullptr;
		i32 size     = 0;
		i32 capacity = 0;


	public:
		PIPE_API BinaryFormatWriter(Arena& arena = p::GetCurrentArena());
		PIPE_API ~BinaryFormatWriter();

		// BEGIN Writer Interface
		PIPE_API void BeginObject() override {}         // Nothing to do
		PIPE_API void BeginArray(u32 size) override;    // Nothing to do
		PIPE_API bool EnterNext(StringView name) override;
		PIPE_API bool EnterNext() override;
		PIPE_API void Leave() override {}    // Nothing to do
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

		PIPE_API TView<p::u8> GetData();

	private:
		void PreAlloc(p::u32 offset);
	};
}    // namespace p