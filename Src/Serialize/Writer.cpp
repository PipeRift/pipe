// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Serialize/Writer.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Serialize/Formats/JsonFormat.h"


namespace p
{
#define WRITER_SWITCH(func)                                                  \
	switch (format)                                                          \
	{                                                                        \
		case SerializeFormat::Json: GetWriter<SerializeFormat::Json>().func; \
	}

#define RETURN_WRITER_SWITCH(func, def)                                             \
	switch (format)                                                                 \
	{                                                                               \
		case SerializeFormat::Json: return GetWriter<SerializeFormat::Json>().func; \
	}                                                                               \
	return def


	// UNCHECKED helper returning a Writer from a format
	template<SerializeFormat inFormat>
	typename FormatBind<inFormat>::Writer& Writer::GetWriter() requires(HasWriter<inFormat>)
	{
		Check(format == inFormat);
		return *static_cast<typename FormatBind<inFormat>::Writer*>(writer);
	}

	// Write a value directly from the format reader.
	template<typename T>
	void WriteFromFormat(Writer& ct, const T& val)
	{
		switch (ct.format)
		{
			case SerializeFormat::Json: ct.GetWriter<SerializeFormat::Json>().Write(val);
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

	void Write(Writer& ct, bool val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(Writer& ct, u8 val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(Writer& ct, i32 val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(Writer& ct, u32 val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(Writer& ct, i64 val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(Writer& ct, u64 val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(Writer& ct, float val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(Writer& ct, double val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(Writer& ct, StringView val)
	{
		WriteFromFormat(ct, val);
	}
}    // namespace p
