// Copyright 2015-2024 Piperift - All rights reserved

#include "PipeSerialize.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/String.h"
#include "Pipe/Reflect/TypeRegistry.h"
#include "PipeMath.h"

#include <yyjson.h>


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
		return false;

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

	void Write(Writer& w, Type* val)
	{
		// TODO: Use name instead of typeId
		w.Serialize(val->GetId());
	}
	void Write(Writer& w, TypeId val)
	{
		w.Serialize(val.GetId());
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
	JsonFormatReader::JsonFormatReader(StringView data)
	{
		InternalInit((char*)data.data(), data.length(), false);
	}

	JsonFormatReader::JsonFormatReader(String& data)
	{
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
		EnsureMsg(scopeStack.Size() == 0,
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
		scopeStack.RemoveAt(scopeStack.Size() - 1, false);
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
		if (!EnsureMsg(yyjson_is_obj(scope.parent),
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
		if (!EnsureMsg(yyjson_is_arr(scope.parent),
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
		if (EnsureMsg(scopeStack.Size() >= 1,
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
			yyjsonAllocator.free(yyjsonAllocator.ctx, const_cast<TChar*>(asString.data()));
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
		scopeStack.RemoveAt(scopeStack.Size() - 1, false);
	}

	bool JsonFormatWriter::EnterNext(StringView name)
	{
		if (!EnsureMsg(yyjson_mut_is_obj(current),
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
		if (!EnsureMsg(yyjson_mut_is_arr(current),
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
		Scope& scope = GetScope();
		if (current) [[unlikely]]
		{
			if (!unsafe_yyjson_is_obj(current)) [[unlikely]]
			{
				CheckMsg(false,
				    "Scope is already initialized but it is not an object. Is BeginObject() being "
				    "mixed with BeginArray() in the same scope?");
			}
			return;
		}

		current = yyjson_mut_obj(doc);
	}

	void JsonFormatWriter::BeginArray(u32 size)
	{
		Scope& scope = GetScope();
		if (current) [[unlikely]]
		{
			if (!unsafe_yyjson_is_arr(current)) [[unlikely]]
			{
				CheckMsg(false,
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
			EnsureMsg(scopeStack.Size() == 0,
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
			yyjsonAllocator.free(yyjsonAllocator.ctx, const_cast<TChar*>(asString.data()));
		}

		yyjson_write_flag flags = pretty ? YYJSON_WRITE_PRETTY : 0;
		sizet size;
		asString =
		    StringView{yyjson_mut_write_opts(doc, flags, &yyjsonAllocator, &size, nullptr), size};
		return asString;
	}
#pragma endregion JsonFormat


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
		val = *pointer;
		++pointer;
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(i8& val)
	{
		val = i8(*pointer);
		++pointer;
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}
	void BinaryFormatReader::Read(u8& val)
	{
		val = *pointer;
		++pointer;
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(i16& val)
	{
		val = pointer[0];
		val |= i16(pointer[1]) << 8;
		pointer += 2;
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(u16& val)
	{
		val = pointer[0];
		val |= u16(pointer[1]) << 8;
		pointer += 2;
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(i32& val)
	{
		val = pointer[0];
		val |= i32(pointer[1]) << 8;
		val |= i32(pointer[2]) << 16;
		val |= i32(pointer[3]) << 24;
		pointer += 4;
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(u32& val)
	{
		val = pointer[0];
		val |= u32(pointer[1]) << 8;
		val |= u32(pointer[2]) << 16;
		val |= u32(pointer[3]) << 24;
		pointer += 4;
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
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
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
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
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(float& val)
	{
		p::CopyMem(&val, pointer, 4);
		pointer += 4;
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(double& val)
	{
		p::CopyMem(&val, pointer, 8);
		pointer += 8;
		CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
	}

	void BinaryFormatReader::Read(StringView& val)
	{
		i32 size = 0;
		Read(size);
		const sizet sizeInBytes = size * sizeof(TChar);
		if (EnsureMsg(pointer + sizeInBytes <= data.EndData(),
		        "The size of a string readen exceeds the read buffer!")) [[likely]]
		{
			val = StringView{reinterpret_cast<TChar*>(pointer), sizeInBytes};
			pointer += sizeInBytes;
			CheckMsg(pointer <= data.EndData(), "The read buffer has been exceeded");
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

	TView<p::u8> BinaryFormatWriter::GetData()
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

			data = static_cast<u8*>(Alloc<u8>(arena, capacity));
			MoveMem(data, oldData, size);
			Free<u8>(arena, oldData, oldCapacity);
		}
	}
#pragma endregion BinaryFormat
}    // namespace p
