// Copyright 2015-2021 Piperift - All rights reserved

#include "Serialization/Contexts/WriteContext.h"

#include "Misc/Checks.h"
#include "Serialization/Formats/JsonFormat.h"


namespace Rift::Serl
{
#define WRITER_SWITCH(func)                 \
	switch (format)                         \
	{                                       \
		case Format::Json:                  \
			GetWriter<Format::Json>().func; \
	}

#define RETURN_WRITER_SWITCH(func, def)            \
	switch (format)                                \
	{                                              \
		case Format::Json:                         \
			return GetWriter<Format::Json>().func; \
	}                                              \
	return def


	// UNCHECKED helper returning a Writer from a format
	template <Format inFormat>
	typename FormatBind<inFormat>::Writer& WriteContext::GetWriter() requires(HasWriter<inFormat>)
	{
		Check(format == inFormat);
		return *static_cast<typename FormatBind<inFormat>::Writer*>(writer);
	}

	// Write a value directly from the format reader.
	template <typename T>
	void WriteFromFormat(WriteContext& ct, const T& val) requires(!ShouldPassByValue<T>)
	{
		switch (ct.format)
		{
			case Format::Json:
				ct.GetWriter<Format::Json>().Write(val);
		}
	}
	template <typename T>
	void WriteFromFormat(WriteContext& ct, T val) requires(ShouldPassByValue<T>)
	{
		switch (ct.format)
		{
			case Format::Json:
				ct.GetWriter<Format::Json>().Write(val);
		}
	}

	void WriteContext::BeginObject()
	{
		WRITER_SWITCH(BeginObject());
	}

	bool WriteContext::EnterNext(StringView name)
	{
		RETURN_WRITER_SWITCH(EnterNext(name), false);
	}

	void WriteContext::BeginArray(u32& size)
	{
		WRITER_SWITCH(BeginArray(size));
	}

	bool WriteContext::EnterNext()
	{
		RETURN_WRITER_SWITCH(EnterNext(), false);
	}

	void WriteContext::Leave()
	{
		WRITER_SWITCH(Leave());
	}

	void Write(WriteContext& ct, bool val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(WriteContext& ct, u8 val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(WriteContext& ct, i32 val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(WriteContext& ct, u32 val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(WriteContext& ct, i64 val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(WriteContext& ct, u64 val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(WriteContext& ct, float val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(WriteContext& ct, double val)
	{
		WriteFromFormat(ct, val);
	}
	void Write(WriteContext& ct, StringView val)
	{
		WriteFromFormat(ct, val);
	}
}    // namespace Rift::Serl
