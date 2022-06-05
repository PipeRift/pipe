// Copyright 2015-2022 Piperift - All rights reserved

#include "Serialization/Contexts/ReadContext.h"

#include "Core/Checks.h"
#include "Core/String.h"
#include "Serialization/Formats/JsonFormat.h"


namespace p::serl
{
#define READER_SWITCH(func)                                \
	switch (format)                                        \
	{                                                      \
		case Format::Json: GetReader<Format::Json>().func; \
	}

#define RETURN_READER_SWITCH(func, def)                           \
	switch (format)                                               \
	{                                                             \
		case Format::Json: return GetReader<Format::Json>().func; \
	}                                                             \
	return def


	// UNCHECKED helper returning a Reader from a format
	template<Format inFormat>
	typename FormatBind<inFormat>::Reader& ReadContext::GetReader() requires(HasReader<inFormat>)
	{
		Check(format == inFormat);
		return *static_cast<typename FormatBind<inFormat>::Reader*>(reader);
	}

	// Read a value directly from the format reader.
	template<typename T>
	void ReadFromFormat(ReadContext& ct, T& val)
	{
		switch (ct.format)
		{
			case Format::Json: ct.GetReader<Format::Json>().Read(val);
		}
	}

	void ReadContext::BeginObject()
	{
		READER_SWITCH(BeginObject());
	}

	bool ReadContext::EnterNext(StringView name)
	{
		RETURN_READER_SWITCH(EnterNext(name), false);
	}

	void ReadContext::BeginArray(u32& size)
	{
		READER_SWITCH(BeginArray(size));
	}

	bool ReadContext::EnterNext()
	{
		RETURN_READER_SWITCH(EnterNext(), false);
	}

	void ReadContext::Leave()
	{
		READER_SWITCH(Leave());
	}

	bool ReadContext::IsObject()
	{
		RETURN_READER_SWITCH(IsObject(), false);
	}

	bool ReadContext::IsArray()
	{
		RETURN_READER_SWITCH(IsArray(), false);
	}

	void Read(ReadContext& ct, bool& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(ReadContext& ct, u8& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(ReadContext& ct, i32& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(ReadContext& ct, u32& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(ReadContext& ct, i64& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(ReadContext& ct, u64& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(ReadContext& ct, float& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(ReadContext& ct, double& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(ReadContext& ct, StringView& val)
	{
		ReadFromFormat(ct, val);
	}
}    // namespace p::serl
