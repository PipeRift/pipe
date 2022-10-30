// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Serialize/Writer.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Serialize/Formats/BinaryFormat.h"
#include "Pipe/Serialize/Formats/JsonFormat.h"


namespace p
{
#define WRITER_SWITCH(func)                                                      \
	switch (format)                                                              \
	{                                                                            \
		case SerializeFormat::Json: GetWriter<SerializeFormat::Json>().func;     \
		case SerializeFormat::Binary: GetWriter<SerializeFormat::Binary>().func; \
	}

#define RETURN_WRITER_SWITCH(func, def)                                                 \
	switch (format)                                                                     \
	{                                                                                   \
		case SerializeFormat::Json: return GetWriter<SerializeFormat::Json>().func;     \
		case SerializeFormat::Binary: return GetWriter<SerializeFormat::Binary>().func; \
	}                                                                                   \
	return def


	// UNCHECKED helper returning a Writer from a format
	template<SerializeFormat inFormat>
	typename FormatBind<inFormat>::Writer& Writer::GetWriter() requires(HasWriter<inFormat>)
	{
		Check(format == inFormat);
		return *static_cast<typename FormatBind<inFormat>::Writer*>(formatWriter);
	}

	// Write a value directly from the format reader.
	template<typename T>
	void WriteFromFormat(Writer& w, const T& val)
	{
		switch (w.format)
		{
			case SerializeFormat::Json: w.GetWriter<SerializeFormat::Json>().Write(val);
			case SerializeFormat::Binary: w.GetWriter<SerializeFormat::Binary>().Write(val);
		}
	}

	void Writer::BeginObject()
	{
		WRITER_SWITCH(BeginObject());
	}

	bool Writer::EnterNext(StringView name)
	{
		RETURN_WRITER_SWITCH(EnterNext(name), false);
	}

	void Writer::BeginArray(u32& size)
	{
		WRITER_SWITCH(BeginArray(size));
	}

	bool Writer::EnterNext()
	{
		RETURN_WRITER_SWITCH(EnterNext(), false);
	}

	void Writer::Leave()
	{
		WRITER_SWITCH(Leave());
	}

	void Write(Writer& w, bool val)
	{
		WriteFromFormat(w, val);
	}
	void Write(Writer& w, u8 val)
	{
		WriteFromFormat(w, val);
	}
	void Write(Writer& w, i32 val)
	{
		WriteFromFormat(w, val);
	}
	void Write(Writer& w, u32 val)
	{
		WriteFromFormat(w, val);
	}
	void Write(Writer& w, i64 val)
	{
		WriteFromFormat(w, val);
	}
	void Write(Writer& w, u64 val)
	{
		WriteFromFormat(w, val);
	}
	void Write(Writer& w, float val)
	{
		WriteFromFormat(w, val);
	}
	void Write(Writer& w, double val)
	{
		WriteFromFormat(w, val);
	}
	void Write(Writer& w, StringView val)
	{
		WriteFromFormat(w, val);
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
