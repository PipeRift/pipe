// Copyright 2015-2023 Piperift - All rights reserved

#include "Pipe/Serialize/Reader.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/String.h"
#include "Pipe/ECS/Id.h"
#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Serialize/Formats/BinaryFormat.h"
#include "Pipe/Serialize/Formats/JsonFormat.h"


namespace p
{
#define READER_SWITCH(func)                                                             \
	switch (format)                                                                     \
	{                                                                                   \
		case SerializeFormat::Json: GetReader<SerializeFormat::Json>().func; break;     \
		case SerializeFormat::Binary: GetReader<SerializeFormat::Binary>().func; break; \
	}

#define RETURN_READER_SWITCH(func, def)                                                        \
	switch (format)                                                                            \
	{                                                                                          \
		case SerializeFormat::Json: return GetReader<SerializeFormat::Json>().func; break;     \
		case SerializeFormat::Binary: return GetReader<SerializeFormat::Binary>().func; break; \
	}                                                                                          \
	return def


	// UNCHECKED helper returning a Reader from a format
	template<SerializeFormat inFormat>
	typename FormatBind<inFormat>::Reader& Reader::GetReader() requires(HasReader<inFormat>)
	{
		Check(format == inFormat);
		return *static_cast<typename FormatBind<inFormat>::Reader*>(formatReader);
	}

	// Read a value directly from the format reader.
	template<typename T>
	void ReadFromFormat(Reader& r, T& val)
	{
		switch (r.format)
		{
			case SerializeFormat::Json: r.GetReader<SerializeFormat::Json>().Read(val); break;
			case SerializeFormat::Binary: r.GetReader<SerializeFormat::Binary>().Read(val); break;
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

	void Read(Reader& r, bool& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, i8& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, u8& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, i16& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, u16& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, i32& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, u32& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, i64& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, u64& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, float& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, double& val)
	{
		ReadFromFormat(r, val);
	}
	void Read(Reader& r, StringView& val)
	{
		ReadFromFormat(r, val);
	}

	void Read(Reader& r, Type*& val)
	{
		// TODO: Use name instead of typeId
		TypeId typeId{};
		r.Serialize(typeId);
		val = TypeRegistry::Get().FindType(typeId);
	}
	void Read(Reader& r, TypeId& val)
	{
		u64 idValue = TypeId::None().GetId();
		r.Serialize(idValue);
		val = TypeId{idValue};
	}
}    // namespace p
