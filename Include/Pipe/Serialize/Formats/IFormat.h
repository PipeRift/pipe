// Copyright 2015-2022 Piperift - All rights reserved
#pragma once

#include "Pipe/Core/Array.h"
#include "Pipe/Core/Platform.h"
#include "Pipe/Core/StringView.h"
#include "Pipe/Serialize/SerializationTypes.h"


namespace p
{
	struct IFormatReader
	{
		PIPE_API virtual void BeginObject()              = 0;
		PIPE_API virtual void BeginArray(u32& size)      = 0;
		PIPE_API virtual bool EnterNext(StringView name) = 0;
		PIPE_API virtual bool EnterNext()                = 0;
		PIPE_API virtual void Leave()                    = 0;
		PIPE_API virtual void Read(bool& val)            = 0;
		PIPE_API virtual void Read(i8& val)              = 0;
		PIPE_API virtual void Read(u8& val)              = 0;
		PIPE_API virtual void Read(i16& val)             = 0;
		PIPE_API virtual void Read(u16& val)             = 0;
		PIPE_API virtual void Read(i32& val)             = 0;
		PIPE_API virtual void Read(u32& val)             = 0;
		PIPE_API virtual void Read(i64& val)             = 0;
		PIPE_API virtual void Read(u64& val)             = 0;
		PIPE_API virtual void Read(float& val)           = 0;
		PIPE_API virtual void Read(double& val)          = 0;
		PIPE_API virtual void Read(StringView& val)      = 0;
		PIPE_API virtual bool IsObject() const           = 0;
		PIPE_API virtual bool IsArray() const            = 0;
		PIPE_API virtual bool IsValid() const            = 0;
	};

	enum PIPE_API WriteFlags
	{
		WriteFlags_None              = 0,
		WriteFlags_CacheStringKeys   = 1 << 0,
		WriteFlags_CacheStringValues = 1 << 1,
		WriteFlags_CacheStrings      = WriteFlags_CacheStringKeys | WriteFlags_CacheStringValues
	};

	struct IFormatWriter
	{
	private:
		sizet activeFlags = WriteFlags_None;
		TArray<sizet> flagStack;

	public:

		PIPE_API virtual void BeginObject()              = 0;
		PIPE_API virtual void BeginArray(u32 size)       = 0;
		PIPE_API virtual bool EnterNext(StringView name) = 0;
		PIPE_API virtual bool EnterNext()                = 0;
		PIPE_API virtual void Leave()                    = 0;
		PIPE_API virtual void Write(bool val)            = 0;
		PIPE_API virtual void Write(i8 val)              = 0;
		PIPE_API virtual void Write(u8 val)              = 0;
		PIPE_API virtual void Write(i16 val)             = 0;
		PIPE_API virtual void Write(u16 val)             = 0;
		PIPE_API virtual void Write(i32 val)             = 0;
		PIPE_API virtual void Write(u32 val)             = 0;
		PIPE_API virtual void Write(i64 val)             = 0;
		PIPE_API virtual void Write(u64 val)             = 0;
		PIPE_API virtual void Write(float val)           = 0;
		PIPE_API virtual void Write(double val)          = 0;
		PIPE_API virtual void Write(StringView val)      = 0;
		PIPE_API virtual bool IsValid() const            = 0;


		PIPE_API void PushAddFlags(WriteFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags |= flags;
		}
		PIPE_API void PushRemoveFlags(WriteFlags flags)
		{
			flagStack.Add(activeFlags);
			activeFlags &= ~flags;
		}
		PIPE_API void PopFlags()
		{
			activeFlags = flagStack.Last();
			flagStack.RemoveAt(flagStack.Size() - 1);
		}

		PIPE_API sizet GetFlags() const
		{
			return activeFlags;
		}
	};


	template<SerializeFormat format>
	struct FormatBind
	{
		using Reader = void;
		using Writer = void;
	};

	template<SerializeFormat format>
	concept HasReader = !IsVoid<typename FormatBind<format>::Reader>;

	template<SerializeFormat format>
	concept HasWriter = !IsVoid<typename FormatBind<format>::Writer>;
}    // namespace p
