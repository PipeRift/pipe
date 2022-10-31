// Copyright 2015-2022 Piperift - All rights reserved
#include "Pipe/Serialize/Formats/BinaryFormat.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/String.h"
#include "Pipe/Math/Math.h"

#include <yyjson.h>


namespace p
{
	BinaryFormatReader::BinaryFormatReader(TSpan<u8> data) : data{data}, pointer{data.Data()} {}

	BinaryFormatReader::~BinaryFormatReader() {}

	void BinaryFormatReader::BeginArray(u32& size)
	{
		Read(size);
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

	void BinaryFormatReader::Read(bool& val)
	{
		CheckMsg(pointer < data.end(), "The read buffer has been exceeded");
		val = *pointer;
		++pointer;
	}

	void BinaryFormatReader::Read(i8& val)
	{
		val = i8(*pointer);
		++pointer;
		CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
	}
	void BinaryFormatReader::Read(u8& val)
	{
		val = *pointer;
		++pointer;
		CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(i16& val)
	{
		val = pointer[0];
		val |= i16(pointer[1]) << 8;
		pointer += 2;
		CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(u16& val)
	{
		val = pointer[0];
		val |= u16(pointer[1]) << 8;
		pointer += 2;
		CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(i32& val)
	{
		val = pointer[0];
		val |= i32(pointer[1]) << 8;
		val |= i32(pointer[2]) << 16;
		val |= i32(pointer[3]) << 24;
		pointer += 4;
		CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(u32& val)
	{
		val = pointer[0];
		val |= u32(pointer[1]) << 8;
		val |= u32(pointer[2]) << 16;
		val |= u32(pointer[3]) << 24;
		pointer += 4;
		CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(i64& val)
	{
		val = pointer[0];
		val |= i64(pointer[1]) << 8;
		val |= i64(pointer[2]) << 16;
		val |= i64(pointer[3]) << 24;
		val |= i64(pointer[4]) << 32;
		val |= i64(pointer[5]) << 40;
		val |= i64(pointer[6]) << 48;
		val |= i64(pointer[7]) << 56;
		pointer += 8;
		CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(u64& val)
	{
		val = pointer[0];
		val |= u64(pointer[1]) << 8;
		val |= u64(pointer[2]) << 16;
		val |= u64(pointer[3]) << 24;
		val |= u64(pointer[4]) << 32;
		val |= u64(pointer[5]) << 40;
		val |= u64(pointer[6]) << 48;
		val |= u64(pointer[7]) << 56;
		pointer += 8;
		CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(float& val)
	{
		p::CopyMem(&val, pointer, 4);
		pointer += 4;
		CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(double& val)
	{
		p::CopyMem(&val, pointer, 8);
		pointer += 8;
		CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(StringView& val)
	{
		i32 size = 0;
		Read(size);
		const sizet sizeInBytes = size * sizeof(TChar);
		if (EnsureMsg(pointer + sizeInBytes <= data.end(),
		        "The size of a string readen exceeds the read buffer!")) [[likely]]
		{
			val = StringView{reinterpret_cast<TChar*>(pointer), sizeInBytes};
			pointer += sizeInBytes;
			CheckMsg(pointer <= data.end(), "The read buffer has been exceeded");
		}
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

	void BinaryFormatWriter::BeginArray(u32 size)
	{
		Write(size);
	}

	bool BinaryFormatWriter::EnterNext(StringView)
	{
		// Nothing to do
		return true;
	}

	bool BinaryFormatWriter::EnterNext()
	{
		// Nothing to do
		return true;
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
