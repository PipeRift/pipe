// Copyright 2015-2026 Piperift. All Rights Reserved.

#include "PipeSerialize.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Guid.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/String.h"
#include "Pipe/Core/Tag.h"
#include "Pipe/Extern/yyjson.h"
#include "Pipe/Extern/ryml.hpp"
#include "Pipe/Extern/fast_float.h"
#include "PipeMath.h"

#include <charconv>


static void* yyjson_malloc(void* ctx, p::sizet size)
{
	return p::HeapAlloc(size);
}
static void* yyjson_realloc(void* ctx, void* ptr, p::sizet oldSize, p::sizet size)
{
	return p::HeapRealloc(ptr, size);
}
static void yyjson_free(void* ctx, void* ptr)
{
	p::HeapFree(ptr);
}
yyjson_alc yyjsonAllocator = {yyjson_malloc, yyjson_realloc, yyjson_free, nullptr};


bool yyjson_mut_obj_add_val(
    yyjson_mut_doc* doc, yyjson_mut_val* obj, p::StringView _key, yyjson_mut_val* _val)
{
	if (yyjson_unlikely(!_val))
	{
		return false;
	}

	if (yyjson_likely(yyjson_mut_is_obj(obj) && _key.data()))
	{
		yyjson_mut_val* key = unsafe_yyjson_mut_val(doc, 2);
		if (yyjson_likely(key))
		{
			size_t len          = unsafe_yyjson_get_len(obj);
			yyjson_mut_val* val = key + 1;
			key->tag            = YYJSON_TYPE_STR | YYJSON_SUBTYPE_NONE;
			key->tag |= (uint64_t)_key.size() << YYJSON_TAG_BIT;
			key->uni.str = _key.data();
			val          = _val;
			unsafe_yyjson_set_len(obj, len + 1);
			unsafe_yyjson_mut_obj_add(obj, key, val, len);
			return true;
		}
	}
	return false;
}


namespace p
{
#pragma region Reader
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
#pragma endregion Reader


#pragma region Writer
	void Writer::BeginObject()
	{
		GetFormat().BeginObject();
	}

	bool Writer::EnterNext(StringView name)
	{
		return GetFormat().EnterNext(name);
	}

	void Writer::BeginArray(u32 size)
	{
		GetFormat().BeginArray(size);
	}

	bool Writer::EnterNext()
	{
		return GetFormat().EnterNext();
	}

	void Writer::Leave()
	{
		GetFormat().Leave();
	}

	void Writer::PushAddFlags(WriteFlags flags)
	{
		GetFormat().PushAddFlags(flags);
	}
	void Writer::PushRemoveFlags(WriteFlags flags)
	{
		GetFormat().PushRemoveFlags(flags);
	}
	void Writer::PopFlags()
	{
		GetFormat().PopFlags();
	}

	void Write(Writer& w, bool val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i8 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u8 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i16 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u16 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i32 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u32 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, i64 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, u64 val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, float val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, double val)
	{
		w.GetFormat().Write(val);
	}
	void Write(Writer& w, StringView val)
	{
		w.GetFormat().Write(val);
	}
#pragma endregion Writer


#pragma region ReadWriter
	void ReadWriter::BeginObject()
	{
		if (IsWriting())
		{
			writer->BeginObject();
		}
		else
		{
			reader->BeginObject();
		}
	}

	bool ReadWriter::EnterNext(StringView name)
	{
		if (IsWriting())
		{
			return writer->EnterNext(name);
		}
		else
		{
			return reader->EnterNext(name);
		}
	}

	void ReadWriter::BeginArray(u32& size)
	{
		if (IsWriting())
		{
			writer->BeginArray(size);
		}
		else
		{
			reader->BeginArray(size);
		}
	}

	bool ReadWriter::EnterNext()
	{
		if (IsWriting())
		{
			return writer->EnterNext();
		}
		else
		{
			return reader->EnterNext();
		}
	}

	void ReadWriter::Leave()
	{
		if (IsWriting())
		{
			writer->Leave();
		}
		else
		{
			reader->Leave();
		}
	}

	void ReadWriter::PushAddFlags(WriteFlags flags)
	{
		if (IsWriting())
		{
			GetWrite().PushAddFlags(flags);
		}
	}

	void ReadWriter::PushRemoveFlags(WriteFlags flags)
	{
		if (IsWriting())
		{
			GetWrite().PushRemoveFlags(flags);
		}
	}

	void ReadWriter::PopFlags()
	{
		if (IsWriting())
		{
			GetWrite().PopFlags();
		}
	}
#pragma endregion ReadWriter


#pragma region JsonFormat
	static constexpr StringView emptyJSON{"{}"};
	JsonFormatReader::JsonFormatReader(StringView data)
	{
		if (data.empty())
		{
			data = emptyJSON;
		}
		InternalInit((char*)data.data(), data.length(), false);
	}

	JsonFormatReader::JsonFormatReader(String& data)
	{
		if (data.empty())
		{
			data.append(emptyJSON);
		}

		// Ensure there is at least 4 bytes of extra memory at the end for insitu reading.
		if (data.capacity() - data.size() < 4)
		{
			data.reserve(data.size() + 4);
		}

		InternalInit(data.data(), data.length(), true);
	}

	JsonFormatReader::~JsonFormatReader()
	{
		if (scopeStack.Size() > 0)
		{
			PopScope();
		}
		P_EnsureMsg(scopeStack.Size() == 0,
		    "Forgot to Leave() some scope? One or more scopes have not been closed.");
		yyjson_doc_free(doc);
	}

	JsonFormatReader::Scope& JsonFormatReader::GetScope()
	{
		return scopeStack.Last();
	}

	void JsonFormatReader::PushScope(yyjson_val* newScope)
	{
		scopeStack.Add({});
		current = newScope;
	}

	void JsonFormatReader::PopScope()
	{
		if (auto* parent = GetScope().parent)
		{
			current = parent;
		}
		scopeStack.RemoveAt(scopeStack.Size() - 1, Shrink::No);
	}

	void JsonFormatReader::BeginObject()
	{
		if (!current) [[unlikely]]
		{
			return;
		}
		InternalBegin();
	}

	void JsonFormatReader::BeginArray(u32& size)
	{
		if (!current) [[unlikely]]
		{
			size = 0;
			return;
		}
		const bool isArray = unsafe_yyjson_is_arr(current);
		size               = InternalBegin();
		if (!isArray)
		{
			size = 0;
		}
	}

	bool JsonFormatReader::EnterNext(StringView name)
	{
		auto& scope = GetScope();
		if (!P_EnsureMsg(yyjson_is_obj(scope.parent),
		        "Current scope is not an object or has not been initialized with BeginObject()"))
		    [[unlikely]]
		{
			return false;
		}

		u32 firstId;
		yyjson_val* firstKey;
		if (scope.id == 0) [[unlikely]]    // Look indexes
		{
			// Start looking from first element
			firstId  = 0;
			firstKey = unsafe_yyjson_get_first(scope.parent);
		}
		else
		{
			// Start looking from next element
			firstId  = scope.id;
			firstKey = unsafe_yyjson_get_next(current);
		}

		if (FindNextKey(firstId, firstKey, name, scope.id, current))
		{
			++scope.id;
			if (scope.id >= scope.size)
			{
				scope.id = 0;
			}
			PushScope(current);
			return true;
		}
		return false;
	}

	bool JsonFormatReader::EnterNext()
	{
		auto& scope = GetScope();
		if (!P_EnsureMsg(yyjson_is_arr(scope.parent),
		        "Current scope is not an array or has not been initialized with BeginArray()"))
		    [[unlikely]]
		{
			return false;
		}

		if (scope.id >= scope.size)
		{
			Error("Tried enter more child scopes than available (Index: {}, Max: {})", scope.id,
			    scope.size);
			return false;
		}

		if (scope.id == 0) [[unlikely]]
		{
			++scope.id;
			PushScope(unsafe_yyjson_get_first(scope.parent));
		}
		else
		{
			++scope.id;
			PushScope(unsafe_yyjson_get_next(current));
		}
		return true;
	}

	void JsonFormatReader::Leave()
	{
		if (P_EnsureMsg(scopeStack.Size() >= 1,
		        "Closed an extra scope! When surrounding EnterScope in if(), make sure to call "
		        "leave scope inside the brackets."))
		{
			PopScope();
		}
	}

	void JsonFormatReader::Read(bool& val)
	{
		val = yyjson_get_bool(current);
	}

	void JsonFormatReader::Read(i8& val)
	{
		if (!yyjson_is_num(current)) [[unlikely]]
		{
			val = 0;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_SINT:
				val = i8(Clamp<i64>(
				    unsafe_yyjson_get_sint(current), Limits<i8>::Lowest(), Limits<i8>::Max()));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i8(Min<u64>(unsafe_yyjson_get_uint(current), Limits<i8>::Max()));
				break;
			case YYJSON_SUBTYPE_REAL: val = i8(unsafe_yyjson_get_real(current)); break;
		}
	}
	void JsonFormatReader::Read(u8& val)
	{
		if (!yyjson_is_num(current)) [[unlikely]]
		{
			val = 0u;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u8(Min<u64>(unsafe_yyjson_get_uint(current), Limits<u8>::Max()));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u8(Clamp<i64>(unsafe_yyjson_get_sint(current), 0, Limits<u8>::Max()));
				break;

			case YYJSON_SUBTYPE_REAL: val = u8(unsafe_yyjson_get_real(current)); break;
		}
	}
	void JsonFormatReader::Read(i16& val)
	{
		if (!yyjson_is_num(current)) [[unlikely]]
		{
			val = 0;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_SINT:
				val = i16(Clamp<i64>(
				    unsafe_yyjson_get_sint(current), Limits<i16>::Lowest(), Limits<i16>::Max()));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i16(Min<u64>(unsafe_yyjson_get_uint(current), Limits<i16>::Max()));
				break;
			case YYJSON_SUBTYPE_REAL: val = i16(unsafe_yyjson_get_real(current)); break;
		}
	}
	void JsonFormatReader::Read(u16& val)
	{
		if (!yyjson_is_num(current)) [[unlikely]]
		{
			val = 0u;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u16(Min<u64>(unsafe_yyjson_get_uint(current), Limits<u16>::Max()));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u16(Clamp<i64>(unsafe_yyjson_get_sint(current), 0, Limits<u16>::Max()));
				break;

			case YYJSON_SUBTYPE_REAL: val = u16(unsafe_yyjson_get_real(current)); break;
		}
	}
	void JsonFormatReader::Read(i32& val)
	{
		if (!yyjson_is_num(current)) [[unlikely]]
		{
			val = 0;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_SINT:
				val = i32(Clamp<i64>(
				    unsafe_yyjson_get_sint(current), Limits<i32>::Lowest(), Limits<i32>::Max()));
				break;
			case YYJSON_SUBTYPE_UINT:
				val = i32(Min<u64>(unsafe_yyjson_get_uint(current), Limits<i32>::Max()));
				break;
			case YYJSON_SUBTYPE_REAL: val = i32(unsafe_yyjson_get_real(current)); break;
		}
	}
	void JsonFormatReader::Read(u32& val)
	{
		if (!yyjson_is_num(current)) [[unlikely]]
		{
			val = 0u;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_UINT:
				val = u32(Min<u64>(unsafe_yyjson_get_uint(current), Limits<u32>::Max()));
				break;
			case YYJSON_SUBTYPE_SINT:
				val = u32(Clamp<i64>(unsafe_yyjson_get_sint(current), 0, Limits<u32>::Max()));
				break;

			case YYJSON_SUBTYPE_REAL: val = u32(unsafe_yyjson_get_real(current)); break;
		}
	}
	void JsonFormatReader::Read(i64& val)
	{
		if (!yyjson_is_num(current)) [[unlikely]]
		{
			val = 0;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_SINT: val = unsafe_yyjson_get_sint(current); break;
			case YYJSON_SUBTYPE_UINT:
				val = i64(Min<u64>(unsafe_yyjson_get_uint(current), Limits<i64>::Max()));
				break;
			case YYJSON_SUBTYPE_REAL: val = i64(unsafe_yyjson_get_real(current)); break;
		}
	}
	void JsonFormatReader::Read(u64& val)
	{
		if (!yyjson_is_num(current)) [[unlikely]]
		{
			val = 0u;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_UINT: val = unsafe_yyjson_get_uint(current); break;
			case YYJSON_SUBTYPE_SINT:
				val = u64(Max<i64>(unsafe_yyjson_get_sint(current), 0));
				break;
			case YYJSON_SUBTYPE_REAL: val = u64(unsafe_yyjson_get_real(current)); break;
		}
	}
	void JsonFormatReader::Read(float& val)
	{
		if (!yyjson_is_num(current)) [[unlikely]]
		{
			val = 0.f;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_REAL: val = float(unsafe_yyjson_get_real(current)); break;
			case YYJSON_SUBTYPE_SINT: val = float(unsafe_yyjson_get_sint(current)); break;
			case YYJSON_SUBTYPE_UINT: val = float(unsafe_yyjson_get_uint(current)); break;
		}
	}
	void JsonFormatReader::Read(double& val)
	{
		if (!yyjson_is_num(current)) [[unlikely]]
		{
			val = 0;    // Default to 0
			return;
		}
		switch (unsafe_yyjson_get_subtype(current))
		{
			case YYJSON_SUBTYPE_REAL: val = unsafe_yyjson_get_real(current); break;
			case YYJSON_SUBTYPE_SINT: val = double(unsafe_yyjson_get_sint(current)); break;
			case YYJSON_SUBTYPE_UINT: val = double(unsafe_yyjson_get_uint(current)); break;
		}
	}
	void JsonFormatReader::Read(StringView& val)
	{
		if (yyjson_is_str(current))
		{
			val = {unsafe_yyjson_get_str(current), unsafe_yyjson_get_len(current)};
			return;
		}
		val = {};
	}

	bool JsonFormatReader::IsObject() const
	{
		return yyjson_is_obj(current);
	}

	bool JsonFormatReader::IsArray() const
	{
		return yyjson_is_arr(current);
	}

	void JsonFormatReader::InternalInit(char* data, sizet size, bool insitu)
	{
		yyjson_read_flag flags = insitu ? YYJSON_READ_INSITU : 0;
		yyjson_read_err err;

		doc = yyjson_read_opts(data, size, flags, &yyjsonAllocator, &err);
		if (doc)
		{
			root = yyjson_doc_get_root(doc);
			PushScope(root);
		}
		else
		{
			error = {ReadErrorCode(err.code), err.msg, err.pos};
		}
	}

	u32 JsonFormatReader::InternalBegin()
	{
		Scope& scope = GetScope();
		if (scope.size > 0) [[unlikely]]
		{
			Error("Have BeginObject() or BeginArray() been called already in this scope?");
			return 0;
		}
		scope.id     = 0;
		scope.size   = u32(unsafe_yyjson_get_len(current));
		scope.parent = current;
		current      = nullptr;
		return scope.size;
	}

	bool JsonFormatReader::FindNextKey(
	    u32 firstId, yyjson_val* firstKey, StringView name, u32& outIndex, yyjson_val*& outValue)
	{
		// Get key of the current value. See yyjson_obj_foreach
		auto* key = firstKey;
		u32 i;
		// Iterate [firstId, last]
		const Scope& scope = GetScope();
		for (i = firstId; i < scope.size; ++i)
		{
			if (unsafe_yyjson_equals_strn(key, name.data(), name.size()))
			{
				outIndex = i;
				outValue = key + 1;
				return true;
			}
			key = unsafe_yyjson_get_next(key + 1);
		}

		// Iterate [first, firstId)
		key = unsafe_yyjson_get_first(scope.parent);
		for (i = 0; i < firstId; ++i)
		{
			if (unsafe_yyjson_equals_strn(key, name.data(), name.size()))
			{
				outIndex = i;
				outValue = key + 1;
				return true;
			}
			key = unsafe_yyjson_get_next(key + 1);
		}

		return false;
	}


	JsonFormatWriter::JsonFormatWriter()
	{
		doc = yyjson_mut_doc_new(nullptr);
		if (doc)
		{
			PushScope({});
		}
	}

	JsonFormatWriter::~JsonFormatWriter()
	{
		Close();
		yyjson_mut_doc_free(doc);

		if (asString.data())
		{
			yyjsonAllocator.free(yyjsonAllocator.ctx, const_cast<char*>(asString.data()));
		}
	}

	JsonFormatWriter::Scope& JsonFormatWriter::GetScope()
	{
		return scopeStack.Last();
	}

	void JsonFormatWriter::PushScope(StringView key)
	{
		if ((GetFlags() & WriteFlags_CacheStringKeys) > 0)
		{
			scopeStack.Add({yyjson_mut_strncpy(doc, key.data(), key.size()), current});
		}
		else
		{
			scopeStack.Add({yyjson_mut_strn(doc, key.data(), key.size()), current});
		}
		current = nullptr;    // New scope
	}

	void JsonFormatWriter::PopScope()
	{
		const Scope& scope = GetScope();
		if (current)
		{
			switch (yyjson_mut_get_type(scope.parent))
			{
				case YYJSON_TYPE_OBJ:
					if (scope.key)
					{
						yyjson_mut_obj_add(scope.parent, scope.key, current);
					}
					break;
				case YYJSON_TYPE_ARR: yyjson_mut_arr_append(scope.parent, current); break;
			}
		}
		current = scope.parent;
		scopeStack.RemoveAt(scopeStack.Size() - 1, Shrink::No);
	}

	bool JsonFormatWriter::EnterNext(StringView name)
	{
		if (!P_EnsureMsg(yyjson_mut_is_obj(current),
		        "Current scope is not an object or has not been initialized with BeginObject()"))
		    [[unlikely]]
		{
			return false;
		}
		PushScope(name);
		return true;
	}

	bool JsonFormatWriter::EnterNext()
	{
		if (!P_EnsureMsg(yyjson_mut_is_arr(current),
		        "Current scope is not an array or has not been initialized with BeginArray()"))
		    [[unlikely]]
		{
			return false;
		}
		PushScope({});
		return true;
	}

	void JsonFormatWriter::Leave()
	{
		PopScope();
	}

	void JsonFormatWriter::BeginObject()
	{
		if (current) [[unlikely]]
		{
			if (!unsafe_yyjson_is_obj(current)) [[unlikely]]
			{
				P_CheckMsg(false,
				    "Scope is already initialized but it is not an object. Is BeginObject() being "
				    "mixed with BeginArray() in the same scope?");
			}
			return;
		}

		current = yyjson_mut_obj(doc);
	}

	void JsonFormatWriter::BeginArray(u32 size)
	{
		if (current) [[unlikely]]
		{
			if (!unsafe_yyjson_is_arr(current)) [[unlikely]]
			{
				P_CheckMsg(false,
				    "Scope is already initialized but it is not an array. Is BeginArray() being "
				    "mixed with BeginObject() in the same scope?");
			}
			return;
		}

		current = yyjson_mut_arr(doc);
	}

	void JsonFormatWriter::Write(bool val)
	{
		current = yyjson_mut_bool(doc, val);
	}
	void JsonFormatWriter::Write(i8 val)
	{
		current = yyjson_mut_sint(doc, val);
	}
	void JsonFormatWriter::Write(u8 val)
	{
		current = yyjson_mut_uint(doc, val);
	}
	void JsonFormatWriter::Write(i16 val)
	{
		current = yyjson_mut_sint(doc, val);
	}
	void JsonFormatWriter::Write(const u16 val)
	{
		current = yyjson_mut_uint(doc, val);
	}
	void JsonFormatWriter::Write(i32 val)
	{
		current = yyjson_mut_sint(doc, val);
	}
	void JsonFormatWriter::Write(const u32 val)
	{
		current = yyjson_mut_uint(doc, val);
	}
	void JsonFormatWriter::Write(i64 val)
	{
		current = yyjson_mut_sint(doc, val);
	}
	void JsonFormatWriter::Write(u64 val)
	{
		current = yyjson_mut_uint(doc, val);
	}
	void JsonFormatWriter::Write(float val)
	{
		current = yyjson_mut_real(doc, val);
	}
	void JsonFormatWriter::Write(double val)
	{
		current = yyjson_mut_real(doc, val);
	}
	void JsonFormatWriter::Write(StringView val)
	{
		if ((GetFlags() & WriteFlags_CacheStringValues) > 0)
		{
			current = yyjson_mut_strncpy(doc, val.data(), val.size());
		}
		else
		{
			current = yyjson_mut_strn(doc, val.data(), val.size());
		}
	}

	void JsonFormatWriter::Close()
	{
		if (open)
		{
			open = false;
			yyjson_mut_doc_set_root(doc, current);

			if (scopeStack.Size() > 0)
			{
				PopScope();
			}
			P_EnsureMsg(scopeStack.Size() == 0,
			    "Forgot to Leave() some scope? One or more scopes have not been closed.");
		}
	}

	StringView JsonFormatWriter::ToString(bool pretty, bool ensureClosed)
	{
		if (ensureClosed)
		{
			Close();
		}

		if (asString.data())
		{
			// Free previous string value
			yyjsonAllocator.free(yyjsonAllocator.ctx, const_cast<char*>(asString.data()));
		}

		yyjson_write_flag flags = pretty ? YYJSON_WRITE_PRETTY : 0;
		sizet size;
		asString =
		    StringView{yyjson_mut_write_opts(doc, flags, &yyjsonAllocator, &size, nullptr), size};
		return asString;
	}
#pragma endregion JsonFormat

#pragma region YamlFormat
	struct YamlFormatReader::Impl
	{
		c4::yml::Tree tree;
		size_t current = c4::yml::NONE;
		struct Scope
		{
			u32 id    = 0;
			u32 size  = 0;
			size_t parent = c4::yml::NONE;
		};
		TArray<Scope> stack;
	};

	struct YamlFormatWriter::Impl
	{
		c4::yml::Tree tree;
		size_t current = c4::yml::NONE;
		struct Scope
		{
			size_t parent = c4::yml::NONE;
			String key;
		};
		TArray<Scope> stack;
	};

	static c4::csubstr ToCSubstr(StringView sv)
	{
		return c4::csubstr(sv.data(), sv.size());
	}

	static StringView ToStringView(c4::csubstr cs)
	{
		return StringView{cs.str, cs.len};
	}

	static size_t YamlFindLogicalRoot(c4::yml::Tree& tree)
	{
		if (tree.empty())
		{
			return c4::yml::NONE;
		}
		const size_t root = tree.root_id();
		if (tree.is_stream(root))
		{
			if (tree.num_children(root) == 0)
			{
				return c4::yml::NONE;
			}
			const size_t doc = tree.doc(0);
			if (tree.is_doc(doc))
			{
				if (tree.has_children(doc))
				{
					const size_t first = tree.first_child(doc);
					if (first != c4::yml::NONE)
					{
						return first;
					}
				}
				return doc;
			}
			return doc;
		}
		return root;
	}

	static bool YamlIsTrue(c4::csubstr s)
	{
		if (s.len == 4)
		{
			return (s.str[0] == 't' || s.str[0] == 'T')
			    && (s.str[1] == 'r' || s.str[1] == 'R')
			    && (s.str[2] == 'u' || s.str[2] == 'U')
			    && (s.str[3] == 'e' || s.str[3] == 'E');
		}
		if (s.len == 1 && s.str[0] == '1')
		{
			return true;
		}
		return false;
	}

	static bool YamlIsFalse(c4::csubstr s)
	{
		if (s.len == 5)
		{
			return (s.str[0] == 'f' || s.str[0] == 'F')
			    && (s.str[1] == 'a' || s.str[1] == 'A')
			    && (s.str[2] == 'l' || s.str[2] == 'L')
			    && (s.str[3] == 's' || s.str[3] == 'S')
			    && (s.str[4] == 'e' || s.str[4] == 'E');
		}
		if (s.len == 1 && s.str[0] == '0')
		{
			return true;
		}
		return false;
	}

	// Reader
	static constexpr StringView emptyYAML{"{}\n"};

	YamlFormatReader::YamlFormatReader(StringView data)
	    : impl{new Impl()}, errorMessage{}
	{
		if (data.empty())
		{
			data = emptyYAML;
		}
		try
		{
			impl->tree = c4::yml::parse_in_arena(ToCSubstr(data));
			const size_t logicalRoot = YamlFindLogicalRoot(impl->tree);
			if (logicalRoot == c4::yml::NONE)
			{
				error = {ReadErrorCode::EmptyContent, "Empty YAML content", 0};
				errorMessage = error.msg ? error.msg : "";
				impl->current = c4::yml::NONE;
				impl->stack.Add({});
			}
			else
			{
				impl->current = logicalRoot;
				impl->stack.Add({});
				error = {ReadErrorCode::InvalidParameter, nullptr, 0};
				error.msg = nullptr;
			}
		}
		catch (const std::exception& e)
		{
			errorMessage = e.what();
			error = {ReadErrorCode::UnexpectedContent, errorMessage.c_str(), 0};
			impl->tree.clear();
			impl->current = c4::yml::NONE;
			if (impl->stack.IsEmpty())
			{
				impl->stack.Add({});
			}
		}
		catch (...)
		{
			errorMessage = "Unknown YAML parse error";
			error = {ReadErrorCode::UnexpectedContent, errorMessage.c_str(), 0};
			impl->tree.clear();
			impl->current = c4::yml::NONE;
			if (impl->stack.IsEmpty())
			{
				impl->stack.Add({});
			}
		}
	}

	YamlFormatReader::YamlFormatReader(String& data)
	    : impl{new Impl()}, errorMessage{}
	{
		if (data.empty())
		{
			data.append(emptyYAML.data(), emptyYAML.size());
		}
		// Use arena parse even for mutable to avoid mutating caller's buffer
		StringView view{data.data(), data.size()};
		try
		{
			impl->tree = c4::yml::parse_in_arena(ToCSubstr(view));
			const size_t logicalRoot = YamlFindLogicalRoot(impl->tree);
			if (logicalRoot == c4::yml::NONE)
			{
				error = {ReadErrorCode::EmptyContent, "Empty YAML content", 0};
				errorMessage = error.msg ? error.msg : "";
				impl->current = c4::yml::NONE;
				impl->stack.Add({});
			}
			else
			{
				impl->current = logicalRoot;
				impl->stack.Add({});
				error = {ReadErrorCode::InvalidParameter, nullptr, 0};
				error.msg = nullptr;
			}
		}
		catch (const std::exception& e)
		{
			errorMessage = e.what();
			error = {ReadErrorCode::UnexpectedContent, errorMessage.c_str(), 0};
			impl->tree.clear();
			impl->current = c4::yml::NONE;
			if (impl->stack.IsEmpty())
			{
				impl->stack.Add({});
			}
		}
		catch (...)
		{
			errorMessage = "Unknown YAML parse error";
			error = {ReadErrorCode::UnexpectedContent, errorMessage.c_str(), 0};
			impl->tree.clear();
			impl->current = c4::yml::NONE;
			if (impl->stack.IsEmpty())
			{
				impl->stack.Add({});
			}
		}
	}

	YamlFormatReader::~YamlFormatReader()
	{
		if (impl)
		{
			if (impl->stack.Size() > 0)
			{
				// Pop all to check for leaks
				while (impl->stack.Size() > 1)
				{
					impl->stack.RemoveAt(impl->stack.Size() - 1, Shrink::No);
				}
				// Leave root scope
				if (impl->stack.Size() == 1)
				{
					impl->stack.RemoveAt(0, Shrink::No);
				}
			}
			P_EnsureMsg(impl->stack.Size() == 0,
			    "Forgot to Leave() some scope? One or more scopes have not been closed.");
			delete impl;
		}
	}

	void YamlFormatReader::BeginObject()
	{
		if (!impl || impl->current == c4::yml::NONE) [[unlikely]]
		{
			return;
		}
		auto& scope = impl->stack.Last();
		if (scope.size != 0) [[unlikely]]
		{
			Error("Have BeginObject() or BeginArray() been called already in this scope?");
			return;
		}
		const size_t node = impl->current;
		// If current is not a map, treat as error but still set size 0
		if (!impl->tree.is_map(node))
		{
			scope.id     = 0;
			scope.size   = 0;
			scope.parent = node;
			impl->current = c4::yml::NONE;
			return;
		}
		scope.id     = 0;
		scope.size   = static_cast<u32>(impl->tree.num_children(node));
		scope.parent = node;
		impl->current = c4::yml::NONE;
	}

	void YamlFormatReader::BeginArray(u32& size)
	{
		if (!impl || impl->current == c4::yml::NONE) [[unlikely]]
		{
			size = 0;
			return;
		}
		const size_t node = impl->current;
		const bool isArray = impl->tree.is_seq(node);
		auto& scope = impl->stack.Last();
		if (scope.size != 0) [[unlikely]]
		{
			Error("Have BeginObject() or BeginArray() been called already in this scope?");
			size = 0;
			return;
		}
		scope.id     = 0;
		scope.size   = static_cast<u32>(impl->tree.num_children(node));
		scope.parent = node;
		impl->current = c4::yml::NONE;
		if (!isArray)
		{
			size = 0;
		}
		else
		{
			size = scope.size;
		}
	}

	bool YamlFormatReader::EnterNext(StringView name)
	{
		if (!impl || impl->stack.IsEmpty()) [[unlikely]]
		{
			return false;
		}
		auto& scope = impl->stack.Last();
		if (scope.parent == c4::yml::NONE || !impl->tree.is_map(scope.parent)) [[unlikely]]
		{
			P_EnsureMsg(false,
			    "Current scope is not an object or has not been initialized with BeginObject()");
			return false;
		}
		const c4::csubstr key = ToCSubstr(name);
		const size_t child = impl->tree.find_child(scope.parent, key);
		if (child == c4::yml::NONE)
		{
			return false;
		}
		++scope.id;
		if (scope.id >= scope.size)
		{
			scope.id = 0;
		}
		impl->stack.Add({});
		impl->current = child;
		return true;
	}

	bool YamlFormatReader::EnterNext()
	{
		if (!impl || impl->stack.IsEmpty()) [[unlikely]]
		{
			return false;
		}
		auto& scope = impl->stack.Last();
		if (scope.parent == c4::yml::NONE || !impl->tree.is_seq(scope.parent)) [[unlikely]]
		{
			P_EnsureMsg(false,
			    "Current scope is not an array or has not been initialized with BeginArray()");
			return false;
		}
		if (scope.id >= scope.size)
		{
			Error("Tried enter more child scopes than available (Index: {}, Max: {})", scope.id,
			    scope.size);
			return false;
		}
		size_t child = c4::yml::NONE;
		if (scope.id == 0)
		{
			child = impl->tree.first_child(scope.parent);
		}
		else
		{
			child = impl->tree.child(scope.parent, scope.id);
		}
		if (child == c4::yml::NONE)
		{
			return false;
		}
		++scope.id;
		impl->stack.Add({});
		impl->current = child;
		return true;
	}

	void YamlFormatReader::Leave()
	{
		if (!impl || !P_EnsureMsg(impl->stack.Size() >= 1,
		           "Closed an extra scope! When surrounding EnterScope in if(), make sure to call "
		           "leave scope inside the brackets."))
		{
			return;
		}
		const size_t parent = impl->stack.Last().parent;
		impl->stack.RemoveAt(impl->stack.Size() - 1, Shrink::No);
		if (parent != c4::yml::NONE)
		{
			impl->current = parent;
		}
		else
		{
			// Leaf scope had no parent, after popping we are inside previous container
			// Mark as none to indicate inside container but not at a specific child
			impl->current = c4::yml::NONE;
			// If stack not empty and its parent is a container, keep current as NONE
			// This matches BeginObject's expectation of NONE inside container
		}
	}

	static c4::csubstr YamlGetVal(const c4::yml::Tree& tree, size_t id)
	{
		if (id == c4::yml::NONE)
		{
			return {};
		}
		if (tree.has_val(id))
		{
			return tree.val(id);
		}
		return {};
	}

	void YamlFormatReader::Read(bool& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = false;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = false;
			return;
		}
		if (YamlIsTrue(s))
		{
			val = true;
		}
		else if (YamlIsFalse(s))
		{
			val = false;
		}
		else
		{
			// Try numeric 1/0
			val = false;
			if (s.len == 1 && s.str[0] == '1')
			{
				val = true;
			}
		}
	}

	template<typename T>
	static bool YamlParseInt(c4::csubstr s, T& out)
	{
		if (s.empty())
		{
			return false;
		}
		// Trim whitespace not needed; ryml already trims
		std::string_view sv(s.str, s.len);
		auto res = std::from_chars(sv.data(), sv.data() + sv.size(), out);
		return res.ec == std::errc{};
	}

	template<typename T>
	static bool YamlParseUInt(c4::csubstr s, T& out)
	{
		if (s.empty())
		{
			return false;
		}
		// Handle negative sign for unsigned: return false
		if (s.str[0] == '-')
		{
			return false;
		}
		std::string_view sv(s.str, s.len);
		auto res = std::from_chars(sv.data(), sv.data() + sv.size(), out);
		return res.ec == std::errc{};
	}

	void YamlFormatReader::Read(i8& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = 0;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = 0;
			return;
		}
		// Try as double first then clamp? Keep simple: try i64 then clamp
		i64 tmp = 0;
		if (YamlParseInt(s, tmp))
		{
			val = i8(Clamp<i64>(tmp, Limits<i8>::Lowest(), Limits<i8>::Max()));
			return;
		}
		double d = 0;
		auto res = fast_float::from_chars(s.str, s.str + s.len, d);
		if (res.ec == std::errc{})
		{
			val = i8(d);
			return;
		}
		val = 0;
	}

	void YamlFormatReader::Read(u8& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = 0;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = 0;
			return;
		}
		u64 tmp = 0;
		if (YamlParseUInt(s, tmp))
		{
			val = u8(Min<u64>(tmp, Limits<u8>::Max()));
			return;
		}
		i64 itmp = 0;
		if (YamlParseInt(s, itmp))
		{
			val = u8(Clamp<i64>(itmp, 0, Limits<u8>::Max()));
			return;
		}
		double d = 0;
		auto res = fast_float::from_chars(s.str, s.str + s.len, d);
		if (res.ec == std::errc{})
		{
			val = u8(d);
			return;
		}
		val = 0;
	}

	void YamlFormatReader::Read(i16& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = 0;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = 0;
			return;
		}
		i64 tmp = 0;
		if (YamlParseInt(s, tmp))
		{
			val = i16(Clamp<i64>(tmp, Limits<i16>::Lowest(), Limits<i16>::Max()));
			return;
		}
		double d = 0;
		auto res = fast_float::from_chars(s.str, s.str + s.len, d);
		if (res.ec == std::errc{})
		{
			val = i16(d);
			return;
		}
		val = 0;
	}

	void YamlFormatReader::Read(u16& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = 0;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = 0;
			return;
		}
		u64 tmp = 0;
		if (YamlParseUInt(s, tmp))
		{
			val = u16(Min<u64>(tmp, Limits<u16>::Max()));
			return;
		}
		i64 itmp = 0;
		if (YamlParseInt(s, itmp))
		{
			val = u16(Clamp<i64>(itmp, 0, Limits<u16>::Max()));
			return;
		}
		double d = 0;
		auto res = fast_float::from_chars(s.str, s.str + s.len, d);
		if (res.ec == std::errc{})
		{
			val = u16(d);
			return;
		}
		val = 0;
	}

	void YamlFormatReader::Read(i32& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = 0;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = 0;
			return;
		}
		i64 tmp = 0;
		if (YamlParseInt(s, tmp))
		{
			val = i32(Clamp<i64>(tmp, Limits<i32>::Lowest(), Limits<i32>::Max()));
			return;
		}
		double d = 0;
		auto res = fast_float::from_chars(s.str, s.str + s.len, d);
		if (res.ec == std::errc{})
		{
			val = i32(d);
			return;
		}
		val = 0;
	}

	void YamlFormatReader::Read(u32& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = 0;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = 0;
			return;
		}
		u64 tmp = 0;
		if (YamlParseUInt(s, tmp))
		{
			val = u32(Min<u64>(tmp, Limits<u32>::Max()));
			return;
		}
		i64 itmp = 0;
		if (YamlParseInt(s, itmp))
		{
			val = u32(Clamp<i64>(itmp, 0, Limits<u32>::Max()));
			return;
		}
		double d = 0;
		auto res = fast_float::from_chars(s.str, s.str + s.len, d);
		if (res.ec == std::errc{})
		{
			val = u32(d);
			return;
		}
		val = 0;
	}

	void YamlFormatReader::Read(i64& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = 0;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = 0;
			return;
		}
		i64 tmp = 0;
		if (YamlParseInt(s, tmp))
		{
			val = tmp;
			return;
		}
		u64 utmp = 0;
		if (YamlParseUInt(s, utmp))
		{
			val = i64(Min<u64>(utmp, Limits<i64>::Max()));
			return;
		}
		double d = 0;
		auto res = fast_float::from_chars(s.str, s.str + s.len, d);
		if (res.ec == std::errc{})
		{
			val = i64(d);
			return;
		}
		val = 0;
	}

	void YamlFormatReader::Read(u64& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = 0;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = 0;
			return;
		}
		u64 tmp = 0;
		if (YamlParseUInt(s, tmp))
		{
			val = tmp;
			return;
		}
		i64 itmp = 0;
		if (YamlParseInt(s, itmp))
		{
			val = u64(Max<i64>(itmp, 0));
			return;
		}
		double d = 0;
		auto res = fast_float::from_chars(s.str, s.str + s.len, d);
		if (res.ec == std::errc{})
		{
			val = u64(d);
			return;
		}
		val = 0;
	}

	void YamlFormatReader::Read(float& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = 0.f;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = 0.f;
			return;
		}
		double d = 0;
		auto res = fast_float::from_chars(s.str, s.str + s.len, d);
		if (res.ec == std::errc{})
		{
			val = float(d);
			return;
		}
		// fallback try int
		i64 tmp = 0;
		if (YamlParseInt(s, tmp))
		{
			val = float(tmp);
			return;
		}
		val = 0.f;
	}

	void YamlFormatReader::Read(double& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = 0;
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.empty())
		{
			val = 0;
			return;
		}
		double d = 0;
		auto res = fast_float::from_chars(s.str, s.str + s.len, d);
		if (res.ec == std::errc{})
		{
			val = d;
			return;
		}
		i64 tmp = 0;
		if (YamlParseInt(s, tmp))
		{
			val = double(tmp);
			return;
		}
		val = 0;
	}

	void YamlFormatReader::Read(StringView& val)
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			val = {};
			return;
		}
		const c4::csubstr s = YamlGetVal(impl->tree, impl->current);
		if (s.len == 0)
		{
			val = {};
			return;
		}
		val = StringView{s.str, s.len};
	}

	bool YamlFormatReader::IsObject() const
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			return false;
		}
		return impl->tree.is_map(impl->current);
	}

	bool YamlFormatReader::IsArray() const
	{
		if (!impl || impl->current == c4::yml::NONE)
		{
			return false;
		}
		return impl->tree.is_seq(impl->current);
	}

	bool YamlFormatReader::IsValid() const
	{
		return impl && impl->current != c4::yml::NONE && !impl->tree.empty();
	}

	// Writer

	YamlFormatWriter::YamlFormatWriter() : impl{new Impl()}, cachedString{}, open{true}
	{
		impl->stack.Add({});
		// current stays NONE until first BeginObject/BeginArray
	}

	YamlFormatWriter::~YamlFormatWriter()
	{
		Close();
		delete impl;
	}

	void YamlFormatWriter::BeginObject()
	{
		if (!impl) [[unlikely]]
		{
			return;
		}
		if (impl->current != c4::yml::NONE) [[unlikely]]
		{
			if (impl->tree.is_map(impl->current))
			{
				return;
			}
			P_CheckMsg(false,
			    "Scope is already initialized but it is not an object. Is BeginObject() being "
			    "mixed with BeginArray() in the same scope?");
			return;
		}
		// current == NONE, need to create map
		if (impl->stack.IsEmpty())
		{
			return;
		}
		auto& scope = impl->stack.Last();
		const size_t parent = scope.parent;
		if (parent == c4::yml::NONE)
		{
			// Top-level
			c4::yml::NodeRef root = impl->tree.rootref();
			if (impl->tree.empty())
			{
				// Tree empty, rootref may still be valid after set_map
				root.set_map();
			}
			else if (!root.is_map())
			{
				// If root is stream/doc, we need to handle. For empty tree we already handled.
				// For parsed tree case not applicable. For writer, tree is new, so root may be SEQ or empty.
				// Force to map if not map
				if (root.is_seq())
				{
					P_CheckMsg(false, "Root is already an array, cannot set to object");
					return;
				}
				root.set_map();
			}
			impl->current = root.id();
		}
		else
		{
			// Create child map under parent
			const size_t child = impl->tree.append_child(parent);
			if (impl->tree.is_map(parent))
			{
				const c4::csubstr key = impl->tree.copy_to_arena(ToCSubstr(scope.key));
				impl->tree.set_key(child, key);
			}
			impl->tree.set_map(child);
			impl->current = child;
		}
	}

	void YamlFormatWriter::BeginArray(u32 /*size*/)
	{
		if (!impl) [[unlikely]]
		{
			return;
		}
		if (impl->current != c4::yml::NONE) [[unlikely]]
		{
			if (impl->tree.is_seq(impl->current))
			{
				return;
			}
			P_CheckMsg(false,
			    "Scope is already initialized but it is not an array. Is BeginArray() being "
			    "mixed with BeginObject() in the same scope?");
			return;
		}
		if (impl->stack.IsEmpty())
		{
			return;
		}
		auto& scope = impl->stack.Last();
		const size_t parent = scope.parent;
		if (parent == c4::yml::NONE)
		{
			c4::yml::NodeRef root = impl->tree.rootref();
			if (impl->tree.empty())
			{
				root.set_seq();
			}
			else if (!root.is_seq())
			{
				if (root.is_map())
				{
					P_CheckMsg(false, "Root is already an object, cannot set to array");
					return;
				}
				root.set_seq();
			}
			impl->current = root.id();
		}
		else
		{
			const size_t child = impl->tree.append_child(parent);
			if (impl->tree.is_map(parent))
			{
				const c4::csubstr key = impl->tree.copy_to_arena(ToCSubstr(scope.key));
				impl->tree.set_key(child, key);
			}
			impl->tree.set_seq(child);
			impl->current = child;
		}
	}

	bool YamlFormatWriter::EnterNext(StringView name)
	{
		if (!impl) [[unlikely]]
		{
			return false;
		}
		if (impl->current == c4::yml::NONE || !impl->tree.is_map(impl->current)) [[unlikely]]
		{
			P_EnsureMsg(false,
			    "Current scope is not an object or has not been initialized with BeginObject()");
			return false;
		}
		typename Impl::Scope s;
		s.parent = impl->current;
		s.key    = String(name.data(), name.size());
		impl->stack.Add(Move(s));
		impl->current = c4::yml::NONE;
		return true;
	}

	bool YamlFormatWriter::EnterNext()
	{
		if (!impl) [[unlikely]]
		{
			return false;
		}
		if (impl->current == c4::yml::NONE || !impl->tree.is_seq(impl->current)) [[unlikely]]
		{
			P_EnsureMsg(false,
			    "Current scope is not an array or has not been initialized with BeginArray()");
			return false;
		}
		typename Impl::Scope s;
		s.parent = impl->current;
		impl->stack.Add(Move(s));
		impl->current = c4::yml::NONE;
		return true;
	}

	void YamlFormatWriter::Leave()
	{
		if (!impl || impl->stack.IsEmpty()) [[unlikely]]
		{
			return;
		}
		const size_t parent = impl->stack.Last().parent;
		impl->stack.RemoveAt(impl->stack.Size() - 1, Shrink::No);
		// After leaving a child, current should be the parent container
		if (parent != c4::yml::NONE)
		{
			impl->current = parent;
		}
		else
		{
			// Leaving top-level child? Keep as is
			if (impl->stack.IsEmpty())
			{
				impl->current = c4::yml::NONE;
			}
		}
	}

	template<typename T>
	static void YamlWriterCreateScalar(YamlFormatWriter::Impl* impl, T val)
	{
		if (!impl || impl->stack.IsEmpty())
		{
			return;
		}
		auto& scope = impl->stack.Last();
		const size_t parent = scope.parent;
		if (parent == c4::yml::NONE)
		{
			// Top-level scalar (no parent map/seq)
			c4::yml::NodeRef root = impl->tree.rootref();
			if (impl->tree.empty())
			{
				// Create root as val
				root.set_val(c4::csubstr{});
			}
			root << val;
			impl->current = root.id();
			return;
		}
		const size_t child = impl->tree.append_child(parent);
		if (impl->tree.is_map(parent))
		{
			const c4::csubstr key = impl->tree.copy_to_arena(ToCSubstr(scope.key));
			impl->tree.set_key(child, key);
		}
		c4::yml::NodeRef ref(impl->tree.tree_ptr(), child);
		// ryml NodeRef constructor takes Tree* and id; but our Impl tree is value, so we need pointer
		// We'll use Tree* directly via &impl->tree
		c4::yml::NodeRef node(&impl->tree, child);
		node << val;
		impl->current = child;
	}

	// Explicit specialization for StringView to ensure arena copy and proper style
	static void YamlWriterCreateString(YamlFormatWriter::Impl* impl, StringView val)
	{
		if (!impl || impl->stack.IsEmpty())
		{
			return;
		}
		auto& scope = impl->stack.Last();
		const size_t parent = scope.parent;
		const c4::csubstr src(val.data(), val.size());
		if (parent == c4::yml::NONE)
		{
			c4::yml::NodeRef root = impl->tree.rootref();
			if (impl->tree.empty())
			{
				root.set_val(c4::csubstr{});
			}
			const c4::csubstr copy = impl->tree.copy_to_arena(src);
			root.set_val(copy);
			impl->current = root.id();
			return;
		}
		const size_t child = impl->tree.append_child(parent);
		if (impl->tree.is_map(parent))
		{
			const c4::csubstr key = impl->tree.copy_to_arena(ToCSubstr(scope.key));
			impl->tree.set_key(child, key);
		}
		const c4::csubstr copy = impl->tree.copy_to_arena(src);
		impl->tree.set_val(child, copy);
		impl->current = child;
	}

	void YamlFormatWriter::Write(bool val)
	{
		YamlWriterCreateScalar(impl, val);
	}
	void YamlFormatWriter::Write(i8 val)
	{
		YamlWriterCreateScalar(impl, int(val));
	}
	void YamlFormatWriter::Write(u8 val)
	{
		YamlWriterCreateScalar(impl, unsigned(val));
	}
	void YamlFormatWriter::Write(i16 val)
	{
		YamlWriterCreateScalar(impl, int(val));
	}
	void YamlFormatWriter::Write(u16 val)
	{
		YamlWriterCreateScalar(impl, unsigned(val));
	}
	void YamlFormatWriter::Write(i32 val)
	{
		YamlWriterCreateScalar(impl, val);
	}
	void YamlFormatWriter::Write(u32 val)
	{
		YamlWriterCreateScalar(impl, val);
	}
	void YamlFormatWriter::Write(i64 val)
	{
		YamlWriterCreateScalar(impl, val);
	}
	void YamlFormatWriter::Write(u64 val)
	{
		YamlWriterCreateScalar(impl, val);
	}
	void YamlFormatWriter::Write(float val)
	{
		YamlWriterCreateScalar(impl, val);
	}
	void YamlFormatWriter::Write(double val)
	{
		YamlWriterCreateScalar(impl, val);
	}
	void YamlFormatWriter::Write(StringView val)
	{
		YamlWriterCreateString(impl, val);
	}

	void YamlFormatWriter::Close()
	{
		if (open)
		{
			open = false;
			// Pop any remaining scopes? In Json, Close pops root scope and checks
			if (impl && impl->stack.Size() > 0)
			{
				// The top-level scope's parent is NONE, its current is root
				// We should pop it to ensure correctness, but keep current as root
				if (impl->stack.Size() == 1 && impl->current != c4::yml::NONE)
				{
					// Top-level scope: just clear stack but keep current
					impl->stack.RemoveAt(0, Shrink::No);
				}
				else
				{
					while (impl->stack.Size() > 0)
					{
						impl->stack.RemoveAt(impl->stack.Size() - 1, Shrink::No);
					}
				}
			}
			P_EnsureMsg(!impl || impl->stack.Size() == 0,
			    "Forgot to Leave() some scope? One or more scopes have not been closed.");
		}
	}

	StringView YamlFormatWriter::ToString(bool /*pretty*/, bool ensureClosed)
	{
		if (ensureClosed)
		{
			Close();
		}
		if (!impl)
		{
			return {};
		}
		cachedString.clear();
		if (impl->tree.empty())
		{
			return {};
		}
		// For consistency with JSON pretty flag, we ignore pretty for YAML (always emit nicely)
		// Use emitrs_yaml to std::string (p::String is std::string)
		c4::yml::emitrs_yaml(impl->tree, &cachedString);
		return StringView{cachedString.data(), cachedString.size()};
	}
#pragma endregion YamlFormat


#pragma region BinaryFormat
	BinaryFormatReader::BinaryFormatReader(TView<u8> data) : data{data}, pointer{data.Data()} {}

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
		val = bool(*pointer);
		++pointer;
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(i8& val)
	{
		val = i8(*pointer);
		++pointer;
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}
	void BinaryFormatReader::Read(u8& val)
	{
		val = *pointer;
		++pointer;
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(i16& val)
	{
		val = pointer[0];
		val |= i16(pointer[1]) << 8;
		pointer += 2;
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(u16& val)
	{
		val = pointer[0];
		val |= u16(pointer[1]) << 8;
		pointer += 2;
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(i32& val)
	{
		val = pointer[0];
		val |= i32(pointer[1]) << 8;
		val |= i32(pointer[2]) << 16;
		val |= i32(pointer[3]) << 24;
		pointer += 4;
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(u32& val)
	{
		val = pointer[0];
		val |= u32(pointer[1]) << 8;
		val |= u32(pointer[2]) << 16;
		val |= u32(pointer[3]) << 24;
		pointer += 4;
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
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
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
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
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(float& val)
	{
		p::CopyMem(&val, pointer, 4);
		pointer += 4;
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(double& val)
	{
		p::CopyMem(&val, pointer, 8);
		pointer += 8;
		P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(StringView& val)
	{
		i32 size = 0;
		Read(size);
		const sizet sizeInBytes = size * sizeof(char);
		if (P_EnsureMsg(pointer + sizeInBytes <= data.EndData(),
		        "The size of a string readen exceeds the read buffer!")) [[likely]]
		{
			val = StringView{reinterpret_cast<char*>(pointer), sizeInBytes};
			pointer += sizeInBytes;
			P_CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
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

	bool BinaryFormatReader::IsValid() const
	{
		return data.Data() && !data.IsEmpty();
	}


	BinaryFormatWriter::BinaryFormatWriter(Arena& arena)
	    : arena{arena}, data{static_cast<u8*>(Alloc<u8>(arena, 64))}, capacity{64}
	{}

	BinaryFormatWriter::~BinaryFormatWriter()
	{
		Free(arena, data, capacity);
	}

	void BinaryFormatWriter::BeginArray(u32 arraySize)
	{
		Write(arraySize);
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
		const i32 valSize = i32(val.size() * sizeof(char));
		PreAlloc(valSize + sizeof(i32));

		Write(i32(val.size()));
		CopyMem(data + size, const_cast<char*>(val.data()), valSize);
		size += valSize;
	}

	TView<p::u8> BinaryFormatWriter::GetData()
	{
		return {data, i32(size)};
	}

	void BinaryFormatWriter::PreAlloc(u32 offset)
	{
		if (size + offset > capacity) [[unlikely]]
		{
			const u32 oldCapacity = capacity;
			capacity *= 2;    // Grow capacity exponentially
			u8* oldData = data;

			data = static_cast<u8*>(Alloc<u8>(arena, capacity));
			MoveMem(data, oldData, size);
			Free<u8>(arena, oldData, oldCapacity);
		}
	}
#pragma endregion BinaryFormat


#pragma region CoreSupport
	void Read(Reader& ct, String& val)
	{
		StringView view;
		ct.Serialize(view);
		val = view;
	}
	void Write(Writer& ct, const String& val)
	{
		ct.Serialize(StringView{val});
	}
	void Read(Reader& ct, Tag& tag)
	{
		StringView str;
		ct.Serialize(str);
		tag = Tag(str);
	}
	void Write(Writer& ct, const Tag& tag)
	{
		ct.Serialize(tag.AsString());
	}

	void Read(Reader& ct, Guid& guid)
	{
		ct.BeginObject();
		ct.Next("a", guid.a);
		ct.Next("b", guid.b);
		ct.Next("c", guid.c);
		ct.Next("d", guid.d);
	}

	void Write(Writer& ct, const Guid& guid)
	{
		ct.BeginObject();
		ct.Next("a", guid.a);
		ct.Next("b", guid.b);
		ct.Next("c", guid.c);
		ct.Next("d", guid.d);
	}

	void Read(Reader& ct, TColor<ColorMode::RGBA>& color)
	{
		u32 value;
		ct.Serialize(value);
		color = TColor<ColorMode::RGBA>::FromHexAlpha(value);
	}
	void Write(Writer& ct, const TColor<ColorMode::RGBA>& color)
	{
		ct.Serialize(color.ToPackedRGBA());
	}
	void Read(Reader& ct, TColor<ColorMode::Linear>& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}
	void Write(Writer& ct, const TColor<ColorMode::Linear>& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}
	void Read(Reader& ct, TColor<ColorMode::sRGB>& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}
	void Write(Writer& ct, const TColor<ColorMode::sRGB>& color)
	{
		ct.BeginObject();
		ct.Next("r", color.r);
		ct.Next("g", color.g);
		ct.Next("b", color.b);
		ct.Next("a", color.a);
	}
	void Read(Reader& r, TColor<ColorMode::HSV>& color)
	{
		r.BeginObject();
		r.Next("h", color.h);
		r.Next("s", color.s);
		r.Next("v", color.v);
		r.Next("a", color.a);
	}
	void Write(Writer& w, const TColor<ColorMode::HSV>& color)
	{
		w.BeginObject();
		w.Next("h", color.h);
		w.Next("s", color.s);
		w.Next("v", color.v);
		w.Next("a", color.a);
	}


	void Read(Reader& ct, Vec<2, float>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Write(Writer& ct, const Vec<2, float>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Read(Reader& ct, Vec<2, u32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Write(Writer& ct, const Vec<2, u32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Read(Reader& ct, Vec<2, i32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Write(Writer& ct, const Vec<2, i32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
	}
	void Read(Reader& ct, Vec<3, float>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Write(Writer& ct, const Vec<3, float>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Read(Reader& ct, Vec<3, u32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Write(Writer& ct, const Vec<3, u32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Read(Reader& ct, Vec<3, i32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Write(Writer& ct, const Vec<3, i32>& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
	}
	void Read(Reader& ct, Quat& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
		ct.Next("w", val.w);
	}
	void Write(Writer& ct, const Quat& val)
	{
		ct.BeginObject();
		ct.Next("x", val.x);
		ct.Next("y", val.y);
		ct.Next("z", val.z);
		ct.Next("w", val.w);
	}
#pragma endregion CoreSupport
}    // namespace p
