// Copyright 2015-2021 Piperift - All rights reserved

#include "Serialization/Formats/JsonFormat.h"
#include "Serialization/ReadContext.h"


namespace Rift::Serl
{
#define READER_SWITCH(func)                  \
	switch (ct.format)                       \
	{                                        \
		case Format_Json:                    \
			GetReader<Format_Json>(ct).func; \
	}

#define RETURN_READER_SWITCH(func, def)             \
	switch (ct.format)                              \
	{                                               \
		case Format_Json:                           \
			return GetReader<Format_Json>(ct).func; \
	}                                               \
	return def


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
		READER_SWITCH(Read(val));
	}

	bool EnterScope(ReadContext& ct, StringView name)
	{
		RETURN_READER_SWITCH(EnterScope(name), false);
	}

	bool EnterScope(ReadContext& ct, u32 index)
	{
		RETURN_READER_SWITCH(EnterScope(index), false);
	}

	void LeaveScope(ReadContext& ct)
	{
		READER_SWITCH(LeaveScope());
	}


	void IterateObject(ReadContext& ct, TFunction<void()> callback)
	{
		READER_SWITCH(IterateObject(callback));
	}

	void IterateObject(ReadContext& ct, TFunction<void(const char*)> callback)
	{
		READER_SWITCH(IterateObject(callback));
	}

	bool IsObject(ReadContext& ct)
	{
		RETURN_READER_SWITCH(IsObject(), false);
	}

	sizet GetObjectSize(ReadContext& ct)
	{
		RETURN_READER_SWITCH(GetObjectSize(), 0);
	}

	void IterateArray(ReadContext& ct, TFunction<void()> callback)
	{
		READER_SWITCH(IterateArray(callback));
	}

	void IterateArray(ReadContext& ct, TFunction<void(u32)> callback)
	{
		READER_SWITCH(IterateArray(callback));
	}

	bool IsArray(ReadContext& ct)
	{
		RETURN_READER_SWITCH(IsArray(), false);
	}

	sizet GetArraySize(ReadContext& ct)
	{
		RETURN_READER_SWITCH(GetArraySize(), 0);
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
