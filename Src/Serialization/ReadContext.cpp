// Copyright 2015-2021 Piperift - All rights reserved

#include "Misc/Checks.h"
#include "Serialization/Formats/JsonFormat.h"
#include "Serialization/ReadContext.h"


namespace Rift::Serl
{
#define READER_SWITCH(ct, func)            \
	switch (format)                        \
	{                                      \
		case Format_Json:                  \
			GetReader<Format_Json>().func; \
	}

#define RETURN_READER_SWITCH(ct, func, def)       \
	switch (format)                               \
	{                                             \
		case Format_Json:                         \
			return GetReader<Format_Json>().func; \
	}                                             \
	return def


	// UNCHECKED helper returning a Reader from a format
	template <Format inFormat>
	typename FormatBind<inFormat>::Reader& ReadContext::GetReader() requires(HasReader<inFormat>)
	{
		Check(format == inFormat);
		return *static_cast<typename FormatBind<inFormat>::Reader*>(reader);
	}

	// Read a value directly from the format reader.
	template <typename T>
	void ReadFromFormat(ReadContext& ct, T& val)
	{
		switch (ct.format)
		{
			case Format_Json:
				ct.GetReader<Format_Json>().Read(val);
		}
	}

	bool ReadContext::EnterNext(StringView name)
	{
		RETURN_READER_SWITCH(*this, EnterNext(name), false);
	}

	bool ReadContext::EnterNext()
	{
		RETURN_READER_SWITCH(*this, EnterNext(), false);
	}

	void ReadContext::Leave()
	{
		READER_SWITCH(*this, Leave());
	}

	bool ReadContext::IsObject()
	{
		RETURN_READER_SWITCH(*this, IsObject(), false);
	}

	bool ReadContext::IsArray()
	{
		RETURN_READER_SWITCH(*this, IsArray(), false);
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
	void Read(ReadContext& ct, String& val)
	{
		StringView view;
		Read(ct, view);
		val = view;
	}
}    // namespace Rift::Serl
