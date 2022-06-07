// Copyright 2015-2022 Piperift - All rights reserved

#include "Pipe/Serialize/Reader.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/String.h"
#include "Pipe/Serialize/Formats/JsonFormat.h"


namespace p
{
#define READER_SWITCH(func)                                                  \
	switch (format)                                                          \
	{                                                                        \
		case SerializeFormat::Json: GetReader<SerializeFormat::Json>().func; \
	}

#define RETURN_READER_SWITCH(func, def)                                             \
	switch (format)                                                                 \
	{                                                                               \
		case SerializeFormat::Json: return GetReader<SerializeFormat::Json>().func; \
	}                                                                               \
	return def


	// UNCHECKED helper returning a Reader from a format
	template<SerializeFormat inFormat>
	typename FormatBind<inFormat>::Reader& Reader::GetReader() requires(HasReader<inFormat>)
	{
		Check(format == inFormat);
		return *static_cast<typename FormatBind<inFormat>::Reader*>(reader);
	}

	// Read a value directly from the format reader.
	template<typename T>
	void ReadFromFormat(Reader& ct, T& val)
	{
		switch (ct.format)
		{
			case SerializeFormat::Json: ct.GetReader<SerializeFormat::Json>().Read(val);
		}
	}

	void Reader::BeginObject()
	{
		READER_SWITCH(BeginObject());
	}

	bool Reader::EnterNext(StringView name)
	{
		RETURN_READER_SWITCH(EnterNext(name), false);
	}

	void Reader::BeginArray(u32& size)
	{
		READER_SWITCH(BeginArray(size));
	}

	bool Reader::EnterNext()
	{
		RETURN_READER_SWITCH(EnterNext(), false);
	}

	void Reader::Leave()
	{
		READER_SWITCH(Leave());
	}

	bool Reader::IsObject()
	{
		RETURN_READER_SWITCH(IsObject(), false);
	}

	bool Reader::IsArray()
	{
		RETURN_READER_SWITCH(IsArray(), false);
	}

	void Read(Reader& ct, bool& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(Reader& ct, u8& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(Reader& ct, i32& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(Reader& ct, u32& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(Reader& ct, i64& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(Reader& ct, u64& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(Reader& ct, float& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(Reader& ct, double& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(Reader& ct, StringView& val)
	{
		ReadFromFormat(ct, val);
	}
}    // namespace p
