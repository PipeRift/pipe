// Copyright 2015-2023 Piperift - All rights reserved
#include "Pipe/Serialize/Formats/JsonFormat.h"

#include "Pipe/Core/Checks.h"
#include "Pipe/Core/Log.h"
#include "Pipe/Core/String.h"
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
	u64 GetKeyTag(sizet size)
	{
		return (((u64)size) << YYJSON_TAG_BIT) | YYJSON_TYPE_STR;
	}
	bool CheckKey(yyjson_val* key, const StringView& name)
	{
		return memcmp(key->uni.ptr, name.data(), name.size()) == 0;
	}

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

		if (doc = yyjson_read_opts(data, size, flags, &yyjsonAllocator, &err))
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
		const Scope& scope = GetScope();
		const u64 tag      = GetKeyTag(name.size());

		// Get key of the current value. See yyjson_obj_foreach
		auto* key = firstKey;
		u32 i;
		// Iterate [firstId, last]
		for (i = firstId; i < scope.size; ++i)
		{
			if (key->tag == tag && CheckKey(key, name))
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
			if (key->tag == tag && CheckKey(key, name))
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
}    // namespace p
