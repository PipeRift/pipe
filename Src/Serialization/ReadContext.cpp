// Copyright 2015-2021 Piperift - All rights reserved

#include "Serialization/Formats/JsonFormat.h"
#include "Serialization/ReadContext.h"


namespace Rift::Serl
{
	// UNCHECKED helper returning a Reader from a format
	template <Format format>
	typename FormatBind<format>::Reader& GetReader(ReadContext& ct) requires(HasReader<format>)
	{
		return *static_cast<typename FormatBind<format>::Reader*>(ct.reader);
	}


	// Read a value directly from the format reader.
	template <typename T>
	void ReadFromFormat(ReadContext& ct, T& val)
	{
		switch (ct.format)
		{
			case Format_Json:
				GetReader<Format_Json>(ct).Read(val);
				break;
			default:
				break;
		}
	}


	void EnterScope(ReadContext& ct, StringView name)
	{
		switch (ct.format)
		{
			case Format_Json:
				GetReader<Format_Json>(ct).EnterScope(name);
				break;
			default:
				break;
		}
	}

	void EnterScope(ReadContext& ct, u32 index)
	{
		switch (ct.format)
		{
			case Format_Json:
				GetReader<Format_Json>(ct).EnterScope(index);
				break;
			default:
				break;
		}
	}

	void LeaveScope(ReadContext& ct)
	{
		switch (ct.format)
		{
			case Format_Json:
				GetReader<Format_Json>(ct).LeaveScope();
				break;
			default:
				break;
		}
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
	void Read(ReadContext& ct, float& val)
	{
		ReadFromFormat(ct, val);
	}
	void Read(ReadContext& ct, String& val)
	{
		ReadFromFormat(ct, val);
	}
}    // namespace Rift::Serl
