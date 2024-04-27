// Copyright 2015-2024 Piperift - All rights reserved

#include "Pipe/Serialize/Reader.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/String.h"
#include "Pipe/Reflect/TypeId.h"
#include "Pipe/Serialize/Formats/BinaryFormat.h"
#include "Pipe/Serialize/Formats/JsonFormat.h"
#include "PipeECS.h"


namespace p
{
	void Reader::BeginObject()
	{
		GetFormat().BeginObject();
	}

	bool Reader::EnterNext(StringView name)
	{
		return GetFormat().EnterNext(name);
	}

	void Reader::BeginArray(u32& size)
	{
		GetFormat().BeginArray(size);
	}

	bool Reader::EnterNext()
	{
		return GetFormat().EnterNext();
	}

	void Reader::Leave()
	{
		GetFormat().Leave();
	}

	bool Reader::IsObject()
	{
		return GetFormat().IsObject();
	}

	bool Reader::IsArray()
	{
		return GetFormat().IsArray();
	}

	void Read(Reader& r, bool& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, i8& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, u8& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, i16& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, u16& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, i32& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, u32& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, i64& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, u64& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, float& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, double& val)
	{
		return r.GetFormat().Read(val);
	}
	void Read(Reader& r, StringView& val)
	{
		return r.GetFormat().Read(val);
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
