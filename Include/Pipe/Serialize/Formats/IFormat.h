// Copyright 2015-2024 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Platform.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Serialize/Reader.h"
#include "Pipe/Serialize/Writer.h"
#include "PipeArrays.h"


namespace p
{
	struct PIPE_API IFormatReader
	{
	private:
		Reader reader{};


	public:
		IFormatReader()
		{
			reader.formatReader = this;
		}

		virtual ~IFormatReader()                = default;
		virtual void BeginObject()              = 0;
		virtual void BeginArray(u32& size)      = 0;
		virtual bool EnterNext(StringView name) = 0;
		virtual bool EnterNext()                = 0;
		virtual void Leave()                    = 0;
		virtual void Read(bool& val)            = 0;
		virtual void Read(i8& val)              = 0;
		virtual void Read(u8& val)              = 0;
		virtual void Read(i16& val)             = 0;
		virtual void Read(u16& val)             = 0;
		virtual void Read(i32& val)             = 0;
		virtual void Read(u32& val)             = 0;
		virtual void Read(i64& val)             = 0;
		virtual void Read(u64& val)             = 0;
		virtual void Read(float& val)           = 0;
		virtual void Read(double& val)          = 0;
		virtual void Read(StringView& val)      = 0;
		virtual bool IsObject() const           = 0;
		virtual bool IsArray() const            = 0;
		virtual bool IsValid() const            = 0;

		Reader& GetReader()
		{
			return reader;
		}
		const Reader& GetReader() const
		{
			return reader;
		}
		operator Reader&()
		{
			return GetReader();
		}
	};


	struct PIPE_API IFormatWriter
	{
	private:
		Writer writer{};


		sizet activeFlags = WriteFlags_None;
		TArray<sizet> flagStack;

	public:
		IFormatWriter()
		{
			writer.formatWriter = this;
		}

		virtual ~IFormatWriter()                = default;
		virtual void BeginObject()              = 0;
		virtual void BeginArray(u32 size)       = 0;
		virtual bool EnterNext(StringView name) = 0;
		virtual bool EnterNext()                = 0;
		virtual void Leave()                    = 0;
		virtual void Write(bool val)            = 0;
		virtual void Write(i8 val)              = 0;
		virtual void Write(u8 val)              = 0;
		virtual void Write(i16 val)             = 0;
		virtual void Write(u16 val)             = 0;
		virtual void Write(i32 val)             = 0;
		virtual void Write(u32 val)             = 0;
		virtual void Write(i64 val)             = 0;
		virtual void Write(u64 val)             = 0;
		virtual void Write(float val)           = 0;
		virtual void Write(double val)          = 0;
		virtual void Write(StringView val)      = 0;
		virtual bool IsValid() const            = 0;


		void PushAddFlags(WriteFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags |= flags;
		}
		void PushRemoveFlags(WriteFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags &= ~flags;
		}
		void PopFlags()
		{
			activeFlags = flagStack.Last();
			flagStack.RemoveAt(flagStack.Size() - 1);
		}

		sizet GetFlags() const
		{
			return activeFlags;
		}

		Writer& GetWriter()
		{
			return writer;
		}
		const Writer& GetWriter() const
		{
			return writer;
		}
		operator Writer&()
		{
			return GetWriter();
		}
	};
}    // namespace p
