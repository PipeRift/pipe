// Copyright 2015-2022 Piperift - All rights reserved
#include "Pipe/Serialize/Formats/BinaryFormat.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/String.h"
#include "Pipe/Math/Math.h"

#include <yyjson.h>


namespace p
{
	BinaryFormatReader::BinaryFormatReader(TSpan<u8> data) : data{data}, pos{data.Data()} {}

	BinaryFormatReader::~BinaryFormatReader() {}

	void BinaryFormatReader::BeginObject()
	{
		// Nothing to do
	}

	void BinaryFormatReader::BeginArray(u32& size)
	{
		NotImplemented;
	}

	bool BinaryFormatReader::EnterNext(StringView)
	{
		// Nothing to do
		return true;
	}

	bool BinaryFormatReader::EnterNext()
	{
		// Nothing to do
		return true;
	}

	void BinaryFormatReader::Leave()
	{
		// Nothing to do
	}

	void BinaryFormatReader::Read(bool& val)
	{
		val = *pos;
		++pos;
	}

	void BinaryFormatReader::Read(i8& val)
	{
		val = *pos;
		++pos;
	}
	void BinaryFormatReader::Read(u8& val)
	{
		val = *pos;
		++pos;
	}

	void BinaryFormatReader::Read(i16& val)
	{
		NotImplemented;
	}

	void BinaryFormatReader::Read(u16& val)
	{
		val = *pos;
		++pos;
	}

	void BinaryFormatReader::Read(i32& val)
	{
		NotImplemented;
	}

	void BinaryFormatReader::Read(u32& val)
	{
		NotImplemented;
	}

	void BinaryFormatReader::Read(i64& val)
	{
		NotImplemented;
	}

	void BinaryFormatReader::Read(u64& val)
	{
		NotImplemented;
	}

	void BinaryFormatReader::Read(float& val)
	{
		NotImplemented;
	}

	void BinaryFormatReader::Read(double& val)
	{
		NotImplemented;
	}

	void BinaryFormatReader::Read(StringView& val)
	{
		NotImplemented;
	}

	bool BinaryFormatReader::IsObject() const
	{
		// Binary format does not track scopes
		return false;
	}

	bool BinaryFormatReader::IsArray() const
	{
		// Binary format does not track scopes
		return false;
	}


	BinaryFormatWriter::BinaryFormatWriter() : data{static_cast<u8*>(Alloc(64))}, capacity{64} {}

	BinaryFormatWriter::~BinaryFormatWriter()
	{
		Free(data, capacity);
	}

	void BinaryFormatWriter::Write(bool val)
	{
		PreAlloc(1);
		data[size] = val;
		++size;
	}
	void BinaryFormatWriter::Write(i8 val)
	{
		PreAlloc(1);
		data[size] = val;
		++size;
	}
	void BinaryFormatWriter::Write(u8 val)
	{
		PreAlloc(1);
		data[size] = val;
		++size;
	}
	void BinaryFormatWriter::Write(i16 val)
	{
		PreAlloc(2);
		u8* p = data + size;
		p[0]  = val & 0xFF;
		p[1]  = val >> 8;
		size += 2;
	}
	void BinaryFormatWriter::Write(u16 val)
	{
		PreAlloc(2);
		u8* p = data + size;
		p[0]  = val & 0xFF;
		p[1]  = val >> 8;
		size += 2;
	}
	void BinaryFormatWriter::Write(i32 val)
	{
		PreAlloc(4);
		u8* p = data + size;
		p[0]  = val & 0xFF;
		p[1]  = (val >> 8) & 0xFF;
		p[2]  = (val >> 16) & 0xFF;
		p[3]  = val >> 24;
		size += 4;
	}
	void BinaryFormatWriter::Write(u32 val)
	{
		PreAlloc(4);
		u8* p = data + size;
		p[0]  = val & 0xFF;
		p[1]  = (val >> 8) & 0xFF;
		p[2]  = (val >> 16) & 0xFF;
		p[3]  = val >> 24;
		size += 4;
	}
	void BinaryFormatWriter::Write(i64 val)
	{
		PreAlloc(8);
		u8* p = data + size;
		p[0]  = val & 0xFF;
		p[1]  = (val >> 8) & 0xFF;
		p[2]  = (val >> 16) & 0xFF;
		p[3]  = (val >> 24) & 0xFF;
		p[4]  = (val >> 32) & 0xFF;
		p[5]  = (val >> 40) & 0xFF;
		p[6]  = (val >> 48) & 0xFF;
		p[7]  = val >> 56;
		size += 8;
	}
	void BinaryFormatWriter::Write(u64 val)
	{
		PreAlloc(8);
		u8* p = data + size;
		p[0]  = val & 0xFF;
		p[1]  = (val >> 8) & 0xFF;
		p[2]  = (val >> 16) & 0xFF;
		p[3]  = (val >> 24) & 0xFF;
		p[4]  = (val >> 32) & 0xFF;
		p[5]  = (val >> 40) & 0xFF;
		p[6]  = (val >> 48) & 0xFF;
		p[7]  = val >> 56;
		size += 8;
	}
	void BinaryFormatWriter::Write(float val)
	{
		PreAlloc(4);
		CopyMem(data + size, &val, 4);
		size += 4;
	}
	void BinaryFormatWriter::Write(double val)
	{
		PreAlloc(8);
		CopyMem(data + size, &val, 8);
		size += 8;
	}
	void BinaryFormatWriter::Write(StringView val)
	{
		const i32 valSize = i32(val.size() * sizeof(TChar));
		PreAlloc(valSize + sizeof(i32));

		Write(i32(val.size()));
		CopyMem(data + size, const_cast<TChar*>(val.data()), valSize);
		size += valSize;
	}

	TSpan<p::u8> BinaryFormatWriter::GetData()
	{
		return {data, size};
	}

	void BinaryFormatWriter::PreAlloc(u32 offset)
	{
		if (size + offset > capacity) [[unlikely]]
		{
			const u32 oldCapacity = capacity;
			capacity *= 2;    // Grow capacity exponentially
			u8* oldData = data;

			data = static_cast<u8*>(Alloc(capacity));
			MoveMem(data, oldData, size);
			Free(oldData, oldCapacity);
		}
	}
}    // namespace p
