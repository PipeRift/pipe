// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Serialize/Writer.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Serialize/Formats/BinaryFormat.h"
#include "Pipe/Serialize/Formats/JsonFormat.h"


namespace p
{
	void Writer::BeginObject()
	{
		GetFormat().BeginObject();
	}

	bool Writer::EnterNext(StringView name)
	{
		return GetFormat().EnterNext(name);
	}

	void Writer::BeginArray(u32 size)
	{
		GetFormat().BeginArray(size);
	}

	bool Writer::EnterNext()
	{
		return GetFormat().EnterNext();
	}

	void Writer::Leave()
	{
		GetFormat().Leave();
	}

	void Writer::PushAddFlags(WriteFlags flags)
	{
		GetFormat().PushAddFlags(flags);
	}
	void Writer::PushRemoveFlags(WriteFlags flags)
	{
		GetFormat().PushRemoveFlags(flags);
	}
	void Writer::PopFlags()
	{
		GetFormat().PopFlags();
	}

	void Write(Writer& w, bool val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i8 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u8 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i16 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u16 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i32 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u32 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i64 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u64 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, float val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, double val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, StringView val)
	{
		w.GetFormat().Write(val);
	}

	void Write(Writer& w, Type* val)
	{
		// TODO: Use name instead of typeId
		w.Serialize(val->GetId());
	}
	void Write(Writer& w, TypeId val)
	{
		w.Serialize(val.GetId());
	}
}    // namespace p
